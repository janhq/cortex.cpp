---
title: Nitro with openai-node
---

You can migrate from OAI API or Azure OpenAI to Nitro using your existing NodeJS code quickly
> The ONLY thing you need to do is to override `baseURL` in `openai` init with `Nitro` URL
- NodeJS OpenAI SDK: https://www.npmjs.com/package/openai

## Chat Completion
<table>
<tr>
<td> Engine </td> <td> Typescript Code </td>
</tr>
<tr>
<td> Nitro </td>
<td>

```typescript
import OpenAI from 'openai';

const openai = new OpenAI({
  apiKey: '', // defaults to process.env["OPENAI_API_KEY"]
  baseURL: "http://localhost:3928/v1/" // https://api.openai.com/v1
});

async function chatCompletion() {
  const stream = await openai.beta.chat.completions.stream({
    model: 'gpt-3.5-turbo',
    messages: [{ role: 'user', content: 'Say this is a test' }],
    stream: true,
  });

  stream.on('content', (delta, snapshot) => {
    process.stdout.write(delta);
  });

  for await (const chunk of stream) {
    process.stdout.write(chunk.choices[0]?.delta?.content || '');
  }

  const chatCompletion = await stream.finalChatCompletion();
  console.log(chatCompletion); // {id: "…", choices: […], …}
}
chatCompletion()
```
</td>
</tr>
<tr>
<td> OAI </td>
<td>

```typescript
import OpenAI from 'openai';

const openai = new OpenAI({
  apiKey: '', // defaults to process.env["OPENAI_API_KEY"]
});

async function chatCompletion() {
  const stream = await openai.beta.chat.completions.stream({
    model: 'gpt-3.5-turbo',
    messages: [{ role: 'user', content: 'Say this is a test' }],
    stream: true,
  });

  stream.on('content', (delta, snapshot) => {
    process.stdout.write(delta);
  });

  for await (const chunk of stream) {
    process.stdout.write(chunk.choices[0]?.delta?.content || '');
  }

  const chatCompletion = await stream.finalChatCompletion();
  console.log(chatCompletion); // {id: "…", choices: […], …}
}
chatCompletion()
```

</td>
</tr>
<tr>
<td> Azure OAI </td>
<td>

```typescript
import OpenAI from 'openai';
// The name of your Azure OpenAI Resource.
// https://learn.microsoft.com/en-us/azure/cognitive-services/openai/how-to/create-resource?pivots=web-portal#create-a-resource
const resource = '<your resource name>';

// Corresponds to your Model deployment within your OpenAI resource, e.g. my-gpt35-16k-deployment
// Navigate to the Azure OpenAI Studio to deploy a model.
const model = '<your model>';

// https://learn.microsoft.com/en-us/azure/ai-services/openai/reference#rest-api-versioning
const apiVersion = '2023-06-01-preview';

const apiKey = process.env['AZURE_OPENAI_API_KEY'];
if (!apiKey) {
  throw new Error('The AZURE_OPENAI_API_KEY environment variable is missing or empty.');
}

const openai = new OpenAI({
  apiKey,
  baseURL: `https://${resource}.openai.azure.com/openai/deployments/${model}`,
  defaultQuery: { 'api-version': apiVersion },
  defaultHeaders: { 'api-key': apiKey },
});

async function chatCompletion() {
  const stream = await openai.beta.chat.completions.stream({
    model: 'gpt-3.5-turbo',
    messages: [{ role: 'user', content: 'Say this is a test' }],
    stream: true,
  });

  stream.on('content', (delta, snapshot) => {
    process.stdout.write(delta);
  });

  for await (const chunk of stream) {
    process.stdout.write(chunk.choices[0]?.delta?.content || '');
  }

  const chatCompletion = await stream.finalChatCompletion();
  console.log(chatCompletion); // {id: "…", choices: […], …}
}
chatCompletion()
```

</td>
</tr>
</table>

## Embedding
<table>
<tr>
<td> Engine </td> <td> Embedding </td>
</tr>
<tr>
<td> Nitro </td>
<td>

```typescript
import OpenAI from 'openai';

const openai = new OpenAI({
  apiKey: '', // defaults to process.env["OPENAI_API_KEY"]
  baseURL: "http://localhost:3928/v1/" // https://api.openai.com/v1
});

async function embedding() {
  const embedding = await openai.embeddings.create({input: 'Hello How are you?', model: 'text-embedding-ada-002'});
  console.log(embedding); // {object: "list", data: […], …}
}

chatCompletion();
```
</td>
</tr>
<tr>
<td> OAI </td>
<td>

```typescript
import OpenAI from 'openai';

const openai = new OpenAI({
  apiKey: '', // defaults to process.env["OPENAI_API_KEY"]
});

async function embedding() {
  const embedding = await openai.embeddings.create({input: 'Hello How are you?', model: 'text-embedding-ada-002'});
  console.log(embedding); // {object: "list", data: […], …}
}

chatCompletion();
```

</td>
</tr>
<tr>
<td> Azure OAI </td>
<td>

```typescript
import OpenAI from 'openai';
// The name of your Azure OpenAI Resource.
// https://learn.microsoft.com/en-us/azure/cognitive-services/openai/how-to/create-resource?pivots=web-portal#create-a-resource
const resource = '<your resource name>';

// Corresponds to your Model deployment within your OpenAI resource, e.g. my-gpt35-16k-deployment
// Navigate to the Azure OpenAI Studio to deploy a model.
const model = '<your model>';

// https://learn.microsoft.com/en-us/azure/ai-services/openai/reference#rest-api-versioning
const apiVersion = '2023-06-01-preview';

const apiKey = process.env['AZURE_OPENAI_API_KEY'];
if (!apiKey) {
  throw new Error('The AZURE_OPENAI_API_KEY environment variable is missing or empty.');
}

const openai = new OpenAI({
  apiKey,
  baseURL: `https://${resource}.openai.azure.com/openai/deployments/${model}`,
  defaultQuery: { 'api-version': apiVersion },
  defaultHeaders: { 'api-key': apiKey },
});

async function embedding() {
  const embedding = await openai.embeddings.create({input: 'Hello How are you?', model: 'text-embedding-ada-002'});
  console.log(embedding); // {object: "list", data: […], …}
}

chatCompletion();
```

</td>
</tr>
</table>

## Audio
Coming soon

## How to reproduce
1. Step 1: Dependencies installation
```
npm install --save openai typescript
# or
yarn add openai
```
2. Step 2: Fill `tsconfig.json`
```json
{
  "compilerOptions": {
    "moduleResolution": "node",
    "sourceMap": true,
    "outDir": "dist",
    "target": "es2020",
    "lib": ["es2020"],
    "module": "commonjs",
  },
  "lib": ["es2015"]
}
```
3. Step 3: Fill `index.ts` file with code
3. Step 4: Build with `npx tsc`
4. Step 5: Run the code with `node dist/index.js`
5. Step 6: Enjoy!