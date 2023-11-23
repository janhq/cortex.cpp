import React from "react";

export default function BuiltWithNitro() {
  return (
    <div className="container">
      <div className="text-center mb-8">
        <h2>Build with Nitro</h2>
        <p className="mt-2 w-full lg:w-2/5 mx-auto">
          Start running local AI models in your app within 10 seconds. Available
          as an npm, pip package, or binary.
        </p>
        <a
          href="/build-source"
          className="px-4 py-2 border border-gray-800 rounded-md inline-block mt-6"
        >
          Developer Docs
        </a>
      </div>

      <div className="w-full lg:w-3/4 mx-auto">
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-10 items-center">
          <div className="relative text-center">
            <img
              src="/img/built/web-app.png"
              alt="Element Web App"
              className="w-3/4 mx-auto h-full object-cover"
            />
            <h4 className="mt-6">Web App</h4>
          </div>
          <div className="relative text-center">
            <img
              src="/img/built/desktop-app.png"
              alt="Element Desktop App"
              className="w-3/4 mx-auto h-full object-cover"
            />
            <h4 className="mt-6">Desktop App</h4>
          </div>
        </div>
      </div>
    </div>
  );
}
