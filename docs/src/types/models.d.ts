export interface Model {
  name: string;
  updated: string;
  command: string;
}

export interface Models {
  cortex_hub: Model[];
  huggingface_hub: Model[];
} 