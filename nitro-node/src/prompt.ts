import { NitroPromptSetting } from "./types";

/**
 * Parse prompt template into agrs settings
 * @param {string} promptTemplate Template as string
 * @returns {(NitroPromptSetting | never)} parsed prompt setting
 * @throws {Error} if cannot split promptTemplate
 */
export function promptTemplateConverter(
  promptTemplate: string,
): NitroPromptSetting | never {
  // Split the string using the markers
  const systemMarker = "{system_message}";
  const promptMarker = "{prompt}";

  if (
    promptTemplate.includes(systemMarker) &&
    promptTemplate.includes(promptMarker)
  ) {
    // Find the indices of the markers
    const systemIndex = promptTemplate.indexOf(systemMarker);
    const promptIndex = promptTemplate.indexOf(promptMarker);

    // Extract the parts of the string
    const system_prompt = promptTemplate.substring(0, systemIndex);
    const user_prompt = promptTemplate.substring(
      systemIndex + systemMarker.length,
      promptIndex,
    );
    const ai_prompt = promptTemplate.substring(
      promptIndex + promptMarker.length,
    );

    // Return the split parts
    return { system_prompt, user_prompt, ai_prompt };
  } else if (promptTemplate.includes(promptMarker)) {
    // Extract the parts of the string for the case where only promptMarker is present
    const promptIndex = promptTemplate.indexOf(promptMarker);
    const user_prompt = promptTemplate.substring(0, promptIndex);
    const ai_prompt = promptTemplate.substring(
      promptIndex + promptMarker.length,
    );

    // Return the split parts
    return { user_prompt, ai_prompt };
  }

  // Throw error if none of the conditions are met
  throw Error("Cannot split prompt template");
}
