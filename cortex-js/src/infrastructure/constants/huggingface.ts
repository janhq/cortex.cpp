export const HUGGING_FACE_TREE_REF_URL = (
  repo: string,
  tree: string,
  path: string,
) => `https://huggingface.co/janhq/${repo}/resolve/${tree}/${path}`;

export const HUGGING_FACE_DOWNLOAD_FILE_MAIN_URL = (
  author: string,
  repo: string,
  fileName: string,
) => `https://huggingface.co/${author}/${repo}/resolve/main/${fileName}`;

export const HUGGING_FACE_REPO_URL = (author: string, repo: string) =>
  `https://huggingface.co/${author}/${repo}`;

export const HUGGING_FACE_REPO_MODEL_API_URL = (repo: string) =>
  `https://huggingface.co/api/models/${repo}`;

export const AllQuantizations = [
  'Q3_K_S',
  'Q3_K_M',
  'Q3_K_L',
  'Q4_K_S',
  'Q4_K_M',
  'Q5_K_S',
  'Q5_K_M',
  'Q4_0',
  'Q4_1',
  'Q5_0',
  'Q5_1',
  'IQ2_XXS',
  'IQ2_XS',
  'Q2_K',
  'Q2_K_S',
  'Q6_K',
  'Q8_0',
  'F16',
  'F32',
  'COPY',
];
