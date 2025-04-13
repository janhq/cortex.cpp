import { defineCollection } from 'astro:content';
import { docsLoader } from '@astrojs/starlight/loaders';
import { docsSchema } from '@astrojs/starlight/schema';
import { z } from 'astro/zod';
import { videosSchema } from 'starlight-videos/schemas'

// Blog collection schema
const blogSchema = z.object({
	title: z.string(),
	date: z.string(),
	author: z.string(),
	image: z.string(),
	description: z.string(),
	tags: z.array(z.string()).default([]),
	draft: z.boolean().default(false)
});

export const collections = {
	docs: defineCollection({ loader: docsLoader(), schema: docsSchema({ extend: videosSchema }) }),
	blog: defineCollection({ schema: blogSchema }),
};
