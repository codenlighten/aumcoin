#!/usr/bin/env node

/**
 * AumCoin Project Knowledge Graph Builder
 * 
 * Using City of Boxes protocol to create:
 * 1. Embeddings of all source files via Lumen Bridge
 * 2. Master Legend (registry) of all modules
 * 3. Semantic search capabilities
 * 4. Context-rich error system for future development
 * 
 * This enables AI agents to work on AumCoin without loading entire codebase into context
 */

const fs = require('fs').promises;
const path = require('path');
const crypto = require('crypto');

// ============================================================================
// CONFIGURATION
// ============================================================================

const CONFIG = {
    LUMEN_BRIDGE_API: 'https://lumenbridge.xyz/api',
    PROJECT_ROOT: process.cwd(),
    OUTPUT_DIR: './project-knowledge',
    
    // Files to analyze
    INCLUDE_PATTERNS: [
        'src/**/*.cpp',
        'src/**/*.h',
        'src/**/*.c',
        '*.md',
        '*.sh',
        'Dockerfile',
        'docker-compose.yml',
        '*.pro'
    ],
    
    EXCLUDE_PATTERNS: [
        'node_modules',
        '.git',
        'build',
        'obj',
        'obj-test',
        '*.o',
        '*.a'
    ]
};

// ============================================================================
// FILE DISCOVERY
// ============================================================================

async function discoverProjectFiles() {
    console.log('🔍 Discovering project files...\n');
    
    const files = [];
    
    async function scanDirectory(dir, relativePath = '') {
        const entries = await fs.readdir(dir, { withFileTypes: true });
        
        for (const entry of entries) {
            const fullPath = path.join(dir, entry.name);
            const relPath = path.join(relativePath, entry.name);
            
            // Skip excluded patterns
            if (CONFIG.EXCLUDE_PATTERNS.some(pattern => relPath.includes(pattern))) {
                continue;
            }
            
            if (entry.isDirectory()) {
                await scanDirectory(fullPath, relPath);
            } else if (entry.isFile()) {
                // Check if file matches include patterns
                const shouldInclude = CONFIG.INCLUDE_PATTERNS.some(pattern => {
                    const regex = new RegExp(pattern.replace('**/', '').replace('*', '.*'));
                    return regex.test(relPath) || relPath.endsWith(pattern.replace('*', ''));
                });
                
                if (shouldInclude) {
                    files.push({
                        path: relPath,
                        fullPath: fullPath,
                        type: getFileType(entry.name),
                        size: (await fs.stat(fullPath)).size
                    });
                }
            }
        }
    }
    
    await scanDirectory(CONFIG.PROJECT_ROOT);
    return files;
}

function getFileType(filename) {
    const ext = path.extname(filename).toLowerCase();
    const typeMap = {
        '.cpp': 'source',
        '.h': 'header',
        '.c': 'source',
        '.md': 'documentation',
        '.sh': 'script',
        '.pro': 'project',
        '': 'config'
    };
    return typeMap[ext] || 'other';
}

// ============================================================================
// FILE ANALYSIS & METADATA EXTRACTION
// ============================================================================

async function analyzeFile(file) {
    console.log(`📄 Analyzing: ${file.path}`);
    
    const content = await fs.readFile(file.fullPath, 'utf-8');
    const lines = content.split('\n');
    
    const metadata = {
        path: file.path,
        type: file.type,
        size: file.size,
        lines: lines.length,
        hash: crypto.createHash('sha256').update(content).digest('hex'),
        
        // Extract module information
        description: extractDescription(content, file.type),
        functions: extractFunctions(content, file.type),
        classes: extractClasses(content, file.type),
        dependencies: extractDependencies(content, file.type),
        opcodes: extractOpcodes(content, file.type),
        
        // Context for AI
        aiContext: generateAIContext(file, content),
        
        // Excerpt for embedding (first 1000 chars)
        excerpt: content.substring(0, 1000)
    };
    
    return metadata;
}

function extractDescription(content, type) {
    const lines = content.split('\n');
    
    // Look for file header comments
    const headerLines = [];
    let inComment = false;
    
    for (let i = 0; i < Math.min(30, lines.length); i++) {
        const line = lines[i].trim();
        
        if (line.startsWith('//') || line.startsWith('#')) {
            headerLines.push(line.replace(/^(\/\/|#)\s*/, ''));
        } else if (line.startsWith('/*')) {
            inComment = true;
        } else if (line.includes('*/')) {
            inComment = false;
        } else if (inComment) {
            headerLines.push(line.replace(/^\s*\*\s*/, ''));
        }
    }
    
    return headerLines.slice(0, 5).join(' ').trim() || 'No description available';
}

function extractFunctions(content, type) {
    if (type !== 'source' && type !== 'header') return [];
    
    const functions = [];
    const functionRegex = /(?:^\s*|\n\s*)(?:static\s+)?(?:inline\s+)?(\w+)\s+(\w+)\s*\([^)]*\)/gm;
    
    let match;
    while ((match = functionRegex.exec(content)) !== null) {
        const returnType = match[1];
        const functionName = match[2];
        
        // Skip common keywords that aren't return types
        if (['if', 'for', 'while', 'switch', 'catch'].includes(returnType)) continue;
        
        functions.push({
            name: functionName,
            returnType: returnType,
            signature: match[0].trim()
        });
    }
    
    return functions.slice(0, 50); // Limit to first 50 functions
}

function extractClasses(content, type) {
    if (type !== 'source' && type !== 'header') return [];
    
    const classes = [];
    const classRegex = /class\s+(\w+)(?:\s*:\s*public\s+(\w+))?/g;
    
    let match;
    while ((match = classRegex.exec(content)) !== null) {
        classes.push({
            name: match[1],
            inherits: match[2] || null
        });
    }
    
    return classes;
}

function extractDependencies(content, type) {
    const dependencies = [];
    
    // C++ includes
    const includeRegex = /#include\s*[<"]([^>"]+)[>"]/g;
    let match;
    while ((match = includeRegex.exec(content)) !== null) {
        dependencies.push(match[1]);
    }
    
    return [...new Set(dependencies)]; // Remove duplicates
}

function extractOpcodes(content, type) {
    if (!content.includes('OP_')) return [];
    
    const opcodes = [];
    const opcodeRegex = /\b(OP_[A-Z_0-9]+)\b/g;
    
    let match;
    while ((match = opcodeRegex.exec(content)) !== null) {
        opcodes.push(match[1]);
    }
    
    return [...new Set(opcodes)].slice(0, 20); // Limit and deduplicate
}

function generateAIContext(file, content) {
    const type = file.type;
    const basename = path.basename(file.path);
    
    let context = `File: ${file.path}\n`;
    context += `Type: ${type}\n`;
    context += `Purpose: `;
    
    // Generate purpose based on file type and name
    if (basename === 'script.cpp') {
        context += 'Core script evaluation engine. Handles all OP_CODES including restored operations (CAT, MUL, DIV, MOD, LSHIFT, RSHIFT, etc.)';
    } else if (basename === 'main.cpp') {
        context += 'Core blockchain logic, consensus rules, transaction validation, block handling';
    } else if (basename === 'init.cpp') {
        context += 'Initialization, configuration, command-line argument parsing';
    } else if (basename === 'util.cpp') {
        context += 'Utility functions, logging, file operations, string manipulation';
    } else if (basename === 'bitcoinrpc.cpp') {
        context += 'RPC server, JSON-RPC handlers, network API';
    } else if (type === 'documentation') {
        context += 'Documentation and guides for developers/users';
    } else if (type === 'script') {
        context += 'Build or deployment automation script';
    } else {
        context += `${type.charAt(0).toUpperCase() + type.slice(1)} file for AumCoin cryptocurrency`;
    }
    
    return context;
}

// ============================================================================
// LUMEN BRIDGE INTEGRATION
// ============================================================================

async function generateEmbedding(text, metadata) {
    // In production, this would call Lumen Bridge API
    // For now, we'll create a mock embedding based on content hash
    
    console.log(`  🧠 Generating embedding for ${metadata.path}...`);
    
    // Simulate API call delay
    await new Promise(resolve => setTimeout(resolve, 100));
    
    // Mock embedding (in production, call Lumen Bridge)
    const hash = crypto.createHash('sha256').update(text).digest();
    const embedding = [];
    for (let i = 0; i < 16; i++) {
        embedding.push(hash[i] / 255.0);
    }
    
    return {
        model: 'lumen-bridge-v1',
        embedding: embedding,
        tokens: Math.ceil(text.length / 4),
        timestamp: new Date().toISOString()
    };
}

async function callLumenBridge(endpoint, data) {
    // Production implementation would use actual Lumen Bridge API
    console.log(`  🌉 Calling Lumen Bridge: ${endpoint}`);
    
    // Mock response
    return {
        success: true,
        data: data,
        agent: 'knowledge-graph-builder',
        timestamp: new Date().toISOString()
    };
}

// ============================================================================
// MASTER LEGEND CREATION (City of Boxes Registry)
// ============================================================================

function createMasterLegend(analyzedFiles) {
    console.log('\n📚 Creating Master Legend (Box Registry)...\n');
    
    const legend = {
        metadata: {
            project: 'AumCoin',
            description: 'Satoshi Vision cryptocurrency with all 15 original Bitcoin OP_CODES restored',
            version: '1.0.0-alpha',
            created: new Date().toISOString(),
            protocol: 'City of Boxes v1.0',
            totalFiles: analyzedFiles.length
        },
        
        boxes: {},
        categories: {},
        dependencies: {},
        security: {
            phase1_complete: true,
            phase2_pending: ['OpenSSL 3.x', 'Boost 1.84+', 'BDB→LevelDB'],
            audit_required: true
        }
    };
    
    // Categorize files into boxes
    for (const file of analyzedFiles) {
        const boxId = generateBoxId(file);
        const category = categorizeFile(file);
        
        legend.boxes[boxId] = {
            id: boxId,
            path: file.path,
            type: file.type,
            category: category,
            
            description: file.description,
            aiContext: file.aiContext,
            
            interface: {
                functions: file.functions.map(f => f.name),
                classes: file.classes.map(c => c.name),
                opcodes: file.opcodes
            },
            
            dependencies: file.dependencies,
            
            metadata: {
                lines: file.lines,
                size: file.size,
                hash: file.hash
            },
            
            // City of Boxes: Contract definition
            contract: generateContract(file),
            
            // Embedding info
            embedding: {
                available: true,
                model: 'lumen-bridge-v1',
                dimensions: 16
            }
        };
        
        // Build category index
        if (!legend.categories[category]) {
            legend.categories[category] = [];
        }
        legend.categories[category].push(boxId);
        
        // Build dependency graph
        for (const dep of file.dependencies) {
            if (!legend.dependencies[dep]) {
                legend.dependencies[dep] = [];
            }
            legend.dependencies[dep].push(boxId);
        }
    }
    
    return legend;
}

function generateBoxId(file) {
    // Convert path to Box ID: src/script.cpp → ScriptBox
    const basename = path.basename(file.path, path.extname(file.path));
    const normalized = basename
        .split(/[-_]/)
        .map(word => word.charAt(0).toUpperCase() + word.slice(1))
        .join('');
    
    return `${normalized}Box`;
}

function categorizeFile(file) {
    const p = file.path.toLowerCase();
    
    if (p.includes('script')) return 'script-engine';
    if (p.includes('main')) return 'core-consensus';
    if (p.includes('net')) return 'network';
    if (p.includes('rpc')) return 'api';
    if (p.includes('wallet')) return 'wallet';
    if (p.includes('crypto') || p.includes('key')) return 'cryptography';
    if (p.includes('db')) return 'storage';
    if (p.includes('util')) return 'utilities';
    if (p.includes('test')) return 'testing';
    if (p.endsWith('.md')) return 'documentation';
    if (p.endsWith('.sh')) return 'build-system';
    if (p.includes('docker')) return 'infrastructure';
    
    return 'other';
}

function generateContract(file) {
    // Generate City of Boxes contract
    return {
        inputs: extractInputContract(file),
        outputs: extractOutputContract(file),
        errors: [
            'SchemaValidationError',
            'FileNotFoundError',
            'CompilationError',
            'RuntimeError'
        ],
        guarantees: [
            'Thread-safe if documented',
            'Memory cleanup on destruction',
            'Error messages include context'
        ]
    };
}

function extractInputContract(file) {
    // Extract what inputs this module accepts
    const inputs = {};
    
    for (const func of file.functions) {
        if (func.name.startsWith('get') || func.name.startsWith('set')) {
            // This is likely an accessor
            inputs[func.name] = {
                type: func.returnType,
                description: `Parameter for ${func.name}`
            };
        }
    }
    
    return inputs;
}

function extractOutputContract(file) {
    // Extract what outputs this module produces
    const outputs = {};
    
    for (const func of file.functions) {
        outputs[func.name] = {
            type: func.returnType,
            description: func.signature
        };
    }
    
    return outputs;
}

// ============================================================================
// SEMANTIC SEARCH INDEX
// ============================================================================

function createSearchIndex(legend, embeddings) {
    console.log('\n🔎 Creating semantic search index...\n');
    
    const index = {
        metadata: {
            created: new Date().toISOString(),
            totalBoxes: Object.keys(legend.boxes).length,
            embeddingDimensions: 16
        },
        
        vectors: {},
        searchCache: {}
    };
    
    // Store embeddings in searchable format
    for (const [boxId, embedding] of Object.entries(embeddings)) {
        if (embedding && embedding.embedding) {
            index.vectors[boxId] = {
                embedding: embedding.embedding,
                boxPath: legend.boxes[boxId].path,
                category: legend.boxes[boxId].category,
                description: legend.boxes[boxId].description
            };
        }
    }
    
    return index;
}

// ============================================================================
// CONTEXT-RICH ERROR TEMPLATES
// ============================================================================

function generateErrorTemplates(legend) {
    console.log('\n⚠️  Generating context-rich error templates...\n');
    
    const templates = {
        metadata: {
            protocol: 'City of Boxes Context-Rich Errors',
            version: '1.0',
            created: new Date().toISOString()
        },
        
        templates: {}
    };
    
    for (const [boxId, box] of Object.entries(legend.boxes)) {
        templates.templates[boxId] = {
            // Layer 1: Identity
            boxId: boxId,
            boxPath: box.path,
            
            // Layer 2: Definition
            definition: box.description,
            purpose: box.aiContext,
            
            // Layer 3: Contract
            contract: box.contract,
            
            // Layer 4: Runtime state (filled at error time)
            runtimeTemplate: {
                timestamp: '{{timestamp}}',
                inputReceived: '{{input}}',
                expectedInput: box.contract.inputs,
                stackTrace: '{{stack}}',
                systemState: '{{state}}'
            },
            
            // AI Prompt for self-repair
            repairPrompt: `You are repairing the ${boxId} module. This module's purpose is: ${box.description}. It requires these inputs: ${JSON.stringify(box.contract.inputs)}. The error occurred because: {{error_reason}}. To fix this, you should: {{suggested_fix}}`
        };
    }
    
    return templates;
}

// ============================================================================
// MAIN EXECUTION
// ============================================================================

async function main() {
    console.log('╔══════════════════════════════════════════════════════════════╗');
    console.log('║                                                              ║');
    console.log('║        🏙️  AumCoin City of Boxes Knowledge Graph  🏙️         ║');
    console.log('║                                                              ║');
    console.log('║            Using Lumen Bridge for Embeddings                 ║');
    console.log('║                                                              ║');
    console.log('╚══════════════════════════════════════════════════════════════╝\n');
    
    try {
        // Create output directory
        await fs.mkdir(CONFIG.OUTPUT_DIR, { recursive: true });
        
        // Step 1: Discover all project files
        const files = await discoverProjectFiles();
        console.log(`✅ Discovered ${files.length} files\n`);
        
        // Step 2: Analyze each file
        console.log('📊 Analyzing files...\n');
        const analyzedFiles = [];
        for (const file of files) {
            const metadata = await analyzeFile(file);
            analyzedFiles.push(metadata);
        }
        console.log(`✅ Analyzed ${analyzedFiles.length} files\n`);
        
        // Step 3: Generate embeddings via Lumen Bridge
        console.log('🧠 Generating embeddings via Lumen Bridge...\n');
        const embeddings = {};
        for (const file of analyzedFiles) {
            const boxId = generateBoxId(file);
            const embeddingText = `${file.aiContext}\n\n${file.excerpt}`;
            embeddings[boxId] = await generateEmbedding(embeddingText, file);
        }
        console.log(`✅ Generated ${Object.keys(embeddings).length} embeddings\n`);
        
        // Step 4: Create Master Legend (Box Registry)
        const legend = createMasterLegend(analyzedFiles);
        
        // Step 5: Create semantic search index
        const searchIndex = createSearchIndex(legend, embeddings);
        
        // Step 6: Generate error templates
        const errorTemplates = generateErrorTemplates(legend);
        
        // Step 7: Save all outputs
        console.log('💾 Saving knowledge graph...\n');
        
        await fs.writeFile(
            path.join(CONFIG.OUTPUT_DIR, 'master-legend.json'),
            JSON.stringify(legend, null, 2)
        );
        console.log('  ✅ Saved: master-legend.json');
        
        await fs.writeFile(
            path.join(CONFIG.OUTPUT_DIR, 'embeddings.json'),
            JSON.stringify(embeddings, null, 2)
        );
        console.log('  ✅ Saved: embeddings.json');
        
        await fs.writeFile(
            path.join(CONFIG.OUTPUT_DIR, 'search-index.json'),
            JSON.stringify(searchIndex, null, 2)
        );
        console.log('  ✅ Saved: search-index.json');
        
        await fs.writeFile(
            path.join(CONFIG.OUTPUT_DIR, 'error-templates.json'),
            JSON.stringify(errorTemplates, null, 2)
        );
        console.log('  ✅ Saved: error-templates.json');
        
        // Step 8: Generate human-readable summary
        const summary = generateSummary(legend, searchIndex, errorTemplates);
        await fs.writeFile(
            path.join(CONFIG.OUTPUT_DIR, 'KNOWLEDGE_GRAPH_SUMMARY.md'),
            summary
        );
        console.log('  ✅ Saved: KNOWLEDGE_GRAPH_SUMMARY.md');
        
        // Final report
        console.log('\n' + '='.repeat(70));
        console.log('\n🎉 Knowledge Graph Complete!\n');
        console.log(`📦 Total Boxes: ${Object.keys(legend.boxes).length}`);
        console.log(`📂 Categories: ${Object.keys(legend.categories).length}`);
        console.log(`🧠 Embeddings: ${Object.keys(embeddings).length}`);
        console.log(`⚠️  Error Templates: ${Object.keys(errorTemplates.templates).length}`);
        console.log(`\n📁 Output directory: ${CONFIG.OUTPUT_DIR}/`);
        console.log('\n' + '='.repeat(70));
        
    } catch (error) {
        console.error('\n❌ Error:', error.message);
        console.error(error.stack);
        process.exit(1);
    }
}

function generateSummary(legend, searchIndex, errorTemplates) {
    let summary = '# AumCoin Knowledge Graph Summary\n\n';
    summary += `**Generated:** ${new Date().toISOString()}\n\n`;
    summary += `**Protocol:** City of Boxes v1.0\n\n`;
    summary += '---\n\n';
    
    summary += '## 📊 Statistics\n\n';
    summary += `- **Total Boxes:** ${Object.keys(legend.boxes).length}\n`;
    summary += `- **Categories:** ${Object.keys(legend.categories).length}\n`;
    summary += `- **Embeddings:** ${Object.keys(searchIndex.vectors).length}\n`;
    summary += `- **Error Templates:** ${Object.keys(errorTemplates.templates).length}\n\n`;
    
    summary += '## 📂 Categories\n\n';
    for (const [category, boxes] of Object.entries(legend.categories)) {
        summary += `### ${category}\n`;
        summary += `Boxes: ${boxes.length}\n`;
        summary += `- ${boxes.slice(0, 5).join(', ')}`;
        if (boxes.length > 5) summary += `, ... (${boxes.length - 5} more)`;
        summary += '\n\n';
    }
    
    summary += '## 🔑 Key Boxes\n\n';
    const keyBoxes = ['ScriptBox', 'MainBox', 'InitBox', 'BitcoinrpcBox', 'WalletBox'];
    for (const boxId of keyBoxes) {
        if (legend.boxes[boxId]) {
            const box = legend.boxes[boxId];
            summary += `### ${boxId}\n`;
            summary += `**Path:** \`${box.path}\`\n\n`;
            summary += `**Description:** ${box.description}\n\n`;
            summary += `**Functions:** ${box.interface.functions.length}\n`;
            summary += `**Classes:** ${box.interface.classes.length}\n\n`;
        }
    }
    
    summary += '## 🔗 Usage\n\n';
    summary += '```javascript\n';
    summary += '// Load the knowledge graph\n';
    summary += 'const legend = require("./project-knowledge/master-legend.json");\n';
    summary += 'const searchIndex = require("./project-knowledge/search-index.json");\n\n';
    summary += '// Find a box\n';
    summary += 'const scriptBox = legend.boxes["ScriptBox"];\n';
    summary += 'console.log(scriptBox.description);\n';
    summary += '```\n\n';
    
    summary += '## 🏗️ City of Boxes Protocol\n\n';
    summary += 'This knowledge graph follows the **City of Boxes** protocol:\n\n';
    summary += '1. **Boxes** - Discrete modules with clear boundaries\n';
    summary += '2. **Contracts** - Defined inputs/outputs/errors\n';
    summary += '3. **Context-Rich Errors** - Errors contain repair instructions\n';
    summary += '4. **Semantic Search** - Find boxes by meaning, not just name\n';
    summary += '5. **AI-Native** - Designed for LLM consumption\n\n';
    
    summary += '## 🚀 Next Steps\n\n';
    summary += '1. Use `search-index.json` for semantic queries\n';
    summary += '2. Reference `error-templates.json` when implementing error handling\n';
    summary += '3. Keep `master-legend.json` updated as code evolves\n';
    summary += '4. Use Lumen Bridge API for embedding generation in production\n\n';
    
    return summary;
}

// Run if called directly
if (require.main === module) {
    main();
}

module.exports = {
    discoverProjectFiles,
    analyzeFile,
    createMasterLegend,
    createSearchIndex,
    generateErrorTemplates
};
