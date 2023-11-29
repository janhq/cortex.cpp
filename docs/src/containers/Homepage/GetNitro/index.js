import React, { useState } from "react";

import { Prism as SyntaxHighlighter } from "react-syntax-highlighter";
import theme from "react-syntax-highlighter/dist/esm/styles/prism/darcula";

import { twMerge } from "tailwind-merge";

import { useClipboard } from "@site/src/hooks/useClipboard";

export default function GetNitro() {
  const userAgent = typeof window !== "undefined" && navigator.userAgent;

  const codeStringShell =
    typeof window !== "undefined" && userAgent?.includes("Windows")
      ? `powershell -Command "& { Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/janhq/nitro/main/install.bat' -OutFile 'install.bat'; .\install.bat; Remove-Item -Path 'install.bat' }"`
      : `curl -sfL https://raw.githubusercontent.com/janhq/nitro/main/install.sh | sudo /bin/bash -`;

  const codeStringNpm = `npm install @janhq/nitro`;
  const codeStringPython = `#(Coming Soon)\npip install @janhq/nitro`;

  const [packageInstall, setPackageInstall] = useState("Shell script");
  const clipboard = useClipboard({ timeout: 500 });

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
    <div className="relative w-full lg:w-1/2 mx-auto mt-10 flex flex-col [&>pre]:!mt-0 [&>pre]:rounded-t-none [&>pre]:border [&>pre]:border-t-0 [&>pre]:border-gray-800 [&>pre]:!bg-[#282A36]">
      <div className="dark:bg-[#09090B]/20 bg-[#F4F4F5]/20 backdrop-blur-md rounded-t-md overflow-hidden border border-b-0 dark:border-gray-800 border-gray-300">
        <ul className="flex">
          {options.map((option, i) => {
            return (
              <li
                key={i}
                className={twMerge(
                  "cursor-pointer px-4 py-2",
                  option === packageInstall && "dark:bg-[#09090B] bg-[#D4D4D8]"
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

      {packageInstall === "Shell script" && (
        <div
          className="absolute top-2 right-4 text-xs px-2 py-1 rounded-md bg-gray-700 text-white cursor-pointer"
          onClick={() => clipboard.copy(renderSyntax(packageInstall))}
        >
          {clipboard.copied ? "Copied" : "Copy"}
        </div>
      )}
    </div>
  );
}
