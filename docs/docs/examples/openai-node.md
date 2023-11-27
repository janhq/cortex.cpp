---
title: Nitro with openai-node
description: Nitro intergration guide for Node.js.
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

const resource = '<your resource name>';
const model = '<your model>';
const apiVersion = '2023-06-01-preview';
const apiKey = process.env['AZURE_OPENAI_API_KEY'];

if (!apiKey) {
  throw new Error('The AZURE_OPENAI_API_KEY variable is missing.');
}

const baseURL = `https://${resource}.openai.azure.com/openai/` +
                `deployments/${model}`;

const openai = new OpenAI({
  apiKey,
  baseURL,
  defaultQuery: { 'api-version': apiVersion },
  defaultHeaders: { 'api-key': apiKey },
});

async function chatCompletion() {
  try {
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
    console.log(chatCompletion); // Log the final completion
  } catch (error) {
    console.error('Error in chat completion:', error);
  }
}

chatCompletion();
```

</td>
</tr>
</table>

> Resource:
> - [Azure Create a resource](https://learn.microsoft.com/en-us/azure/cognitive-services/openai/how-to/create-resource?pivots=web-portal#create-a-resource)
> - [Azure-OAI Rest API versoning](https://learn.microsoft.com/en-us/azure/ai-services/openai/reference#rest-api-versioning)

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
  apiKey: '', // Defaults to process.env["OPENAI_API_KEY"]
  baseURL: 'http://localhost:3928/v1/'
  // 'https://api.openai.com/v1'
});

async function embedding() {
  try {
    const response = await openai.embeddings.create({
      input: 'Hello How are you?',
      model: 'text-embedding-ada-002'
    });
    console.log(response); // Log the response
  } catch (error) {
    console.error('Error in fetching embedding:', error);
  }
}

embedding();
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
  const embedding = await openai.embeddings.create({
    input: 'Hello How are you?',
    model: 'text-embedding-ada-002'
  });
  console.log(embedding); // {object: "list", data: […], …}
}

embedding();
```

</td>
</tr>
<tr>
<td> Azure OAI </td>
<td>

```typescript
import OpenAI from 'openai';

const resource = '<your resource name>';
const model = '<your model>';
const apiVersion = '2023-06-01-preview';
const apiKey = process.env['AZURE_OPENAI_API_KEY'];

if (!apiKey) {
  throw new Error('The AZURE_OPENAI_API_KEY variable is missing.');
}

// Splitting the baseURL into concatenated parts for readability
const baseURL = `https://${resource}.openai.azure.com/openai/` +
                `deployments/${model}`;

const openai = new OpenAI({
  apiKey,
  baseURL,
  defaultQuery: { 'api-version': apiVersion },
  defaultHeaders: { 'api-key': apiKey },
});

async function embedding() {
  const embedding = await openai.embeddings.create({
    input: 'Hello How are you?',
    model: 'text-embedding-ada-002'
  });
  console.log(embedding); // {object: "list", data: […], …}
}

embedding();
```

</td>
</tr>
</table>

## Audio

:::info Coming soon
:::

## How to reproduce

**Step 1:** Dependencies installation

```bash
npm install --save openai typescript
# or
yarn add openai
```

**Step 2:** Fill `tsconfig.json`

```js
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

**Step 3:** Fill `index.ts` file with code.

**Step 4:** Build with `npx tsc`.

**Step 5:** Run the code with `node dist/index.js`.