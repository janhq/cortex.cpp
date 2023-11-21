import React, { useState } from "react";

import { Prism as SyntaxHighlighter } from "react-syntax-highlighter";
import theme from "react-syntax-highlighter/dist/esm/styles/prism/darcula";

import { twMerge } from "tailwind-merge";

import { useClipboard } from "@site/src/hooks/useClipboard";

export default function GetNitro() {
  const codeStringShell = `curl -fsSL https://nitro.jan.ai/install -o get-nitro.sh`;
  const codeStringNpm = `npm install nitro`;
  const codeStringPython = `pip install nitro`;

  const [packageInstall, setPackageInstall] = useState("Shell script");
  const clipboard = useClipboard();

  const options = ["Shell script", "NPM", "Python"];

  const renderSyntax = (option) => {
    switch (option) {
      case "NPM":
        return codeStringNpm;

      case "Python":
        return codeStringPython;

      default:
        return codeStringShell;
    }
  };

  return (
    <div className="relative w-full lg:w-1/2 mx-auto mt-10 flex flex-col [&>pre]:!mt-0 [&>pre]:rounded-t-none [&>pre]:border [&>pre]:border-t-0 [&>pre]:border-gray-800">
      <div className="bg-[#09090B]/20 backdrop-blur-md rounded-t-md overflow-hidden border border-b-0 border-gray-800">
        <ul className="flex">
          {options.map((option, i) => {
            return (
              <li
                key={i}
                className={twMerge(
                  "cursor-pointer px-4 py-2",
                  option === packageInstall && "bg-[#18181B]"
                )}
                onClick={() => setPackageInstall(option)}
              >
                {option}
              </li>
            );
          })}
        </ul>
      </div>
      <SyntaxHighlighter language="bash" style={theme}>
        {renderSyntax(packageInstall)}
      </SyntaxHighlighter>

      <div
        className="absolute bottom-5 right-4 text-xs px-2 py-1 rounded-md bg-gray-700 cursor-pointer"
        onClick={() => clipboard.copy(renderSyntax(packageInstall))}
      >
        {clipboard.copied ? "Copied" : "Copy"}
      </div>
    </div>
  );
}
