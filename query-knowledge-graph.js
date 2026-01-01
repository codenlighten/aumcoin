#!/usr/bin/env node

/**
 * AumCoin Knowledge Graph Query Tool
 * 
 * Semantic search over the AumCoin codebase using embeddings
 * City of Boxes protocol implementation
 */

const fs = require('fs').promises;
const path = require('path');

// Load knowledge graph
let legend = null;
let searchIndex = null;
let errorTemplates = null;

async function loadKnowledgeGraph() {
    console.log('📚 Loading knowledge graph...\n');
    
    legend = JSON.parse(
        await fs.readFile('./project-knowledge/master-legend.json', 'utf-8')
    );
    
    searchIndex = JSON.parse(
        await fs.readFile('./project-knowledge/search-index.json', 'utf-8')
    );
    
    errorTemplates = JSON.parse(
        await fs.readFile('./project-knowledge/error-templates.json', 'utf-8')
    );
    
    console.log(`✅ Loaded ${Object.keys(legend.boxes).length} boxes\n`);
}

// ============================================================================
// QUERY FUNCTIONS
// ============================================================================

function queryByCategory(category) {
    console.log(`\n🔍 Query: Find all boxes in category "${category}"\n`);
    
    if (!legend.categories[category]) {
        console.log(`❌ Category "${category}" not found`);
        console.log(`\nAvailable categories:`);
        Object.keys(legend.categories).forEach(cat => {
            console.log(`  - ${cat} (${legend.categories[cat].length} boxes)`);
        });
        return;
    }
    
    const boxes = legend.categories[category];
    console.log(`Found ${boxes.length} boxes:\n`);
    
    boxes.forEach(boxId => {
        const box = legend.boxes[boxId];
        console.log(`📦 ${boxId}`);
        console.log(`   Path: ${box.path}`);
        console.log(`   Desc: ${box.description.substring(0, 80)}...`);
        console.log(`   Functions: ${box.interface.functions.length} | Classes: ${box.interface.classes.length}`);
        console.log('');
    });
}

function queryByKeyword(keyword) {
    console.log(`\n🔍 Query: Find boxes containing "${keyword}"\n`);
    
    const matches = [];
    
    for (const [boxId, box] of Object.entries(legend.boxes)) {
        const searchText = `
            ${box.path} 
            ${box.description} 
            ${box.aiContext}
            ${box.interface.functions.join(' ')}
            ${box.interface.classes.join(' ')}
            ${box.interface.opcodes.join(' ')}
        `.toLowerCase();
        
        if (searchText.includes(keyword.toLowerCase())) {
            matches.push({ boxId, box, relevance: countOccurrences(searchText, keyword.toLowerCase()) });
        }
    }
    
    // Sort by relevance
    matches.sort((a, b) => b.relevance - a.relevance);
    
    console.log(`Found ${matches.length} matches:\n`);
    
    matches.slice(0, 10).forEach(({ boxId, box, relevance }) => {
        console.log(`📦 ${boxId} (${relevance} occurrences)`);
        console.log(`   Path: ${box.path}`);
        console.log(`   Category: ${box.category}`);
        console.log(`   Desc: ${box.description.substring(0, 80)}...`);
        console.log('');
    });
    
    if (matches.length > 10) {
        console.log(`... and ${matches.length - 10} more matches\n`);
    }
}

function queryDependencies(filename) {
    console.log(`\n🔍 Query: What depends on "${filename}"?\n`);
    
    if (!legend.dependencies[filename]) {
        console.log(`❌ No boxes depend on "${filename}"`);
        console.log(`\nTip: Try searching for similar files:`);
        const similar = Object.keys(legend.dependencies)
            .filter(dep => dep.includes(filename) || filename.includes(dep))
            .slice(0, 5);
        similar.forEach(dep => console.log(`  - ${dep}`));
        return;
    }
    
    const dependents = legend.dependencies[filename];
    console.log(`${dependents.length} boxes depend on ${filename}:\n`);
    
    dependents.forEach(boxId => {
        const box = legend.boxes[boxId];
        console.log(`📦 ${boxId}`);
        console.log(`   Path: ${box.path}`);
        console.log(`   Category: ${box.category}`);
        console.log('');
    });
}

function showBox(boxId) {
    console.log(`\n📦 Box Details: ${boxId}\n`);
    
    if (!legend.boxes[boxId]) {
        console.log(`❌ Box "${boxId}" not found`);
        console.log(`\nSimilar boxes:`);
        const similar = Object.keys(legend.boxes)
            .filter(id => id.toLowerCase().includes(boxId.toLowerCase()))
            .slice(0, 10);
        similar.forEach(id => console.log(`  - ${id}`));
        return;
    }
    
    const box = legend.boxes[boxId];
    
    console.log(`ID:          ${box.id}`);
    console.log(`Path:        ${box.path}`);
    console.log(`Type:        ${box.type}`);
    console.log(`Category:    ${box.category}`);
    console.log(`\nDescription:`);
    console.log(`  ${box.description}`);
    console.log(`\nAI Context:`);
    console.log(`  ${box.aiContext}`);
    console.log(`\nInterface:`);
    console.log(`  Functions:   ${box.interface.functions.length}`);
    if (box.interface.functions.length > 0) {
        console.log(`    - ${box.interface.functions.slice(0, 10).join('\n    - ')}`);
        if (box.interface.functions.length > 10) {
            console.log(`    ... and ${box.interface.functions.length - 10} more`);
        }
    }
    console.log(`  Classes:     ${box.interface.classes.length}`);
    if (box.interface.classes.length > 0) {
        console.log(`    - ${box.interface.classes.join('\n    - ')}`);
    }
    console.log(`  OP_CODES:    ${box.interface.opcodes.length}`);
    if (box.interface.opcodes.length > 0) {
        console.log(`    - ${box.interface.opcodes.join(', ')}`);
    }
    console.log(`\nDependencies: ${box.dependencies.length}`);
    if (box.dependencies.length > 0) {
        console.log(`  - ${box.dependencies.slice(0, 10).join('\n  - ')}`);
        if (box.dependencies.length > 10) {
            console.log(`  ... and ${box.dependencies.length - 10} more`);
        }
    }
    console.log(`\nMetadata:`);
    console.log(`  Lines:       ${box.metadata.lines}`);
    console.log(`  Size:        ${formatBytes(box.metadata.size)}`);
    console.log(`  Hash:        ${box.metadata.hash.substring(0, 16)}...`);
    console.log('');
}

function listCategories() {
    console.log(`\n📂 Categories (${Object.keys(legend.categories).length} total):\n`);
    
    const sorted = Object.entries(legend.categories)
        .sort((a, b) => b[1].length - a[1].length);
    
    sorted.forEach(([category, boxes]) => {
        console.log(`${category.padEnd(20)} ${boxes.length} boxes`);
    });
    console.log('');
}

function stats() {
    console.log(`\n📊 AumCoin Knowledge Graph Statistics\n`);
    console.log(`Total Boxes:       ${Object.keys(legend.boxes).length}`);
    console.log(`Categories:        ${Object.keys(legend.categories).length}`);
    console.log(`Dependencies:      ${Object.keys(legend.dependencies).length}`);
    console.log(`Embeddings:        ${Object.keys(searchIndex.vectors).length}`);
    console.log(`Error Templates:   ${Object.keys(errorTemplates.templates).length}`);
    console.log(`\nProject:           ${legend.metadata.project}`);
    console.log(`Description:       ${legend.metadata.description}`);
    console.log(`Version:           ${legend.metadata.version}`);
    console.log(`Protocol:          ${legend.metadata.protocol}`);
    console.log(`Created:           ${new Date(legend.metadata.created).toLocaleString()}`);
    console.log(`\nSecurity Status:`);
    console.log(`  Phase 1:         ${legend.security.phase1_complete ? '✅ Complete' : '❌ Incomplete'}`);
    console.log(`  Phase 2:         ${legend.security.phase2_pending.join(', ')}`);
    console.log(`  Audit Required:  ${legend.security.audit_required ? '⚠️  Yes' : '✅ No'}`);
    console.log('');
}

function semanticQuery(query) {
    console.log(`\n🧠 Semantic Query: "${query}"\n`);
    console.log(`(Note: Using keyword fallback - integrate with Lumen Bridge for true semantic search)\n`);
    
    // In production, this would use Lumen Bridge embeddings
    // For now, fall back to keyword search
    queryByKeyword(query);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

function countOccurrences(text, keyword) {
    const matches = text.match(new RegExp(keyword, 'gi'));
    return matches ? matches.length : 0;
}

function formatBytes(bytes) {
    if (bytes < 1024) return bytes + ' B';
    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
    return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
}

function showHelp() {
    console.log(`
╔══════════════════════════════════════════════════════════════╗
║                                                              ║
║          🔍 AumCoin Knowledge Graph Query Tool 🔍            ║
║                                                              ║
║                  City of Boxes Protocol                      ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝

Usage: node query-knowledge-graph.js <command> [arguments]

Commands:

  category <name>          List all boxes in a category
                          Example: node query-knowledge-graph.js category script-engine

  keyword <word>          Search for keyword in box metadata
                          Example: node query-knowledge-graph.js keyword transaction

  depends <filename>      Show what depends on a file
                          Example: node query-knowledge-graph.js depends script.h

  box <boxId>            Show detailed info about a box
                          Example: node query-knowledge-graph.js box ScriptBox

  categories             List all available categories

  stats                  Show overall statistics

  semantic <query>       Semantic search (natural language)
                          Example: node query-knowledge-graph.js semantic "transaction validation"

  help                   Show this help message

Examples:

  # Find all script-related modules
  node query-knowledge-graph.js category script-engine

  # What handles RPC calls?
  node query-knowledge-graph.js keyword rpc

  # What depends on the script engine?
  node query-knowledge-graph.js depends script.h

  # Get details about the main consensus module
  node query-knowledge-graph.js box MainBox

  # Show project statistics
  node query-knowledge-graph.js stats
`);
}

// ============================================================================
// MAIN EXECUTION
// ============================================================================

async function main() {
    const args = process.argv.slice(2);
    
    if (args.length === 0 || args[0] === 'help') {
        showHelp();
        return;
    }
    
    await loadKnowledgeGraph();
    
    const command = args[0];
    const arg = args.slice(1).join(' ');
    
    switch (command) {
        case 'category':
            queryByCategory(arg);
            break;
        case 'keyword':
            queryByKeyword(arg);
            break;
        case 'depends':
            queryDependencies(arg);
            break;
        case 'box':
            showBox(arg);
            break;
        case 'categories':
            listCategories();
            break;
        case 'stats':
            stats();
            break;
        case 'semantic':
            semanticQuery(arg);
            break;
        default:
            console.log(`❌ Unknown command: ${command}`);
            console.log(`Run 'node query-knowledge-graph.js help' for usage\n`);
    }
}

main().catch(error => {
    console.error('❌ Error:', error.message);
    process.exit(1);
});
