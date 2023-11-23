import React from "react";
import { FaGithub } from "react-icons/fa";

export default function GithubButtob() {
  return (
    <a
      href="https://github.com/janhq/nitro/releases"
      className="text-white bg-blue-600 hover:bg-blue-700 hover:text-white inline-flex px-4 py-3 rounded-lg text-lg font-semibold cursor-pointer justify-center items-center space-x-2"
    >
      <span>
        <FaGithub />
      </span>
      <span>View on Github</span>
    </a>
  );
}
