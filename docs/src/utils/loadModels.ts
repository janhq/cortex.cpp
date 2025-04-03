import yaml from 'js-yaml';
import fs from 'fs';
import path from 'path';
import type { Models } from '../types/models';

export function loadModels(): Models {
  const filePath = path.join(process.cwd(), 'src/data/models.yaml');
  const fileContents = fs.readFileSync(filePath, 'utf8');
  return yaml.load(fileContents) as Models;
} 