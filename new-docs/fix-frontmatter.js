#!/usr/bin/env bun

import { readdir, readFile, writeFile } from 'fs/promises';
import { join, basename } from 'path';

const contentDir = './src/content/docs';

function toTitleCase(str) {
  // Convert kebab or snake case to space-separated title case
  return str
    .replace(/[-_]/g, ' ')
    .split(' ')
    .map(word => word.charAt(0).toUpperCase() + word.slice(1))
    .join(' ');
}

async function fixFrontmatter(filePath) {
  try {
    const content = await readFile(filePath, 'utf8');
    
    if (content.startsWith('---\nunlisted: true\n---')) {
      // Extract the base filename without extension
      const fileName = basename(filePath).replace(/\.(md|mdx)$/, '');
      
      // Create title from filename (convert hyphen to space and capitalize)
      const title = toTitleCase(fileName);
      
      // Replace frontmatter with title
      const updatedContent = content.replace(
        '---\nunlisted: true\n---', 
        `---\ntitle: ${title}\nunlisted: true\n---`
      );
      
      await writeFile(filePath, updatedContent);
      console.log(`Updated frontmatter in ${filePath}`);
      return true;
    }
    
    return false;
  } catch (error) {
    console.error(`Error processing ${filePath}:`, error);
    return false;
  }
}

async function processDirectory(dir) {
  const entries = await readdir(dir, { withFileTypes: true });
  let totalUpdated = 0;
  
  for (const entry of entries) {
    const entryPath = join(dir, entry.name);
    
    if (entry.isDirectory()) {
      totalUpdated += await processDirectory(entryPath);
    } else if ((entry.name.endsWith('.md') || entry.name.endsWith('.mdx'))) {
      const updated = await fixFrontmatter(entryPath);
      if (updated) totalUpdated++;
    }
  }
  
  return totalUpdated;
}

async function main() {
  try {
    console.log('Starting frontmatter fix...');
    const updatedCount = await processDirectory(contentDir);
    console.log(`Done! Updated ${updatedCount} files.`);
  } catch (error) {
    console.error('Error:', error);
  }
}

main();