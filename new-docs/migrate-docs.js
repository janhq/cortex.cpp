#!/usr/bin/env bun

import { readdir, stat, mkdir, readFile, writeFile, copyFile as fsCopyFile } from 'fs/promises';
import { join, dirname, resolve } from 'path';

// Source and destination directories
const sourceDir = resolve(import.meta.dir, '../docs/docs');
const destDir = resolve(import.meta.dir, './src/content/docs');

// Function to ensure directories exist
async function ensureDir(dir) {
  try {
    await stat(dir);
  } catch (e) {
    await mkdir(dir, { recursive: true });
    console.log(`Created directory: ${dir}`);
  }
}

// Function to convert frontmatter if needed (Docusaurus to Astro)
function convertFrontmatter(content) {
  // Docusaurus uses --- for frontmatter, which is compatible with Astro
  // Just checking if there are any specific conversions needed
  return content;
}

// Function to copy and transform a file
async function copyFile(sourcePath, destPath) {
  // Read the source file
  const content = await readFile(sourcePath, 'utf8');
  
  // Transform content if needed
  const transformedContent = convertFrontmatter(content);
  
  // Ensure the destination directory exists
  await ensureDir(dirname(destPath));
  
  // Write the transformed content to the destination file
  await writeFile(destPath, transformedContent);
  console.log(`Copied: ${sourcePath} -> ${destPath}`);
}

// Function to recursively copy files
async function copyFiles(source, dest) {
  const entries = await readdir(source, { withFileTypes: true });
  
  await ensureDir(dest);
  
  for (const entry of entries) {
    const sourcePath = join(source, entry.name);
    const destPath = join(dest, entry.name);
    
    if (entry.isDirectory()) {
      await copyFiles(sourcePath, destPath);
    } else if (entry.name.endsWith('.md') || entry.name.endsWith('.mdx')) {
      await copyFile(sourcePath, destPath);
    } else {
      // Copy other files directly
      await fsCopyFile(sourcePath, destPath);
      console.log(`Copied file: ${sourcePath} -> ${destPath}`);
    }
  }
}

// Main function
async function main() {
  try {
    console.log('Starting migration from Docusaurus to Astro...');
    console.log(`Source: ${sourceDir}`);
    console.log(`Destination: ${destDir}`);
    
    await copyFiles(sourceDir, destDir);
    
    console.log('Migration completed successfully!');
  } catch (error) {
    console.error('Error during migration:', error);
    process.exit(1);
  }
}

main();