import React from "react";

const Announcement = () => {
  return (
    <div className="inline-flex items-center justify-center alert px-4 py-3 rounded-xl mx-auto bg-indigo-500 border border-solid border-indigo-800">
      {/* Please change this when cortex stable we can use from latest release endpoint */}
      <div className="flex items-center gap-2">
        <span>🎉</span>
        <p className="mb-0 text-neutral-100 font-medium">
          <a href="https://github.com/menloresearch/cortex.cpp/releases/tag/v1.0.1" className="no-underline hover:no-underline">
            {" "}
            Cortex.cpp v1.0 is now live on GitHub. Check it out!
          </a>
        </p>
      </div>
    </div>
  );
};

export default Announcement;
