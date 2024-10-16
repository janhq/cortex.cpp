import BrowserOnly from "@docusaurus/BrowserOnly";
import { ExcalidrawImperativeAPI } from "@excalidraw/excalidraw/types/types";

interface DiagramProps {
  diagramPath: string;
}

export default function Diagram({ diagramPath }: DiagramProps) {
  return (
    <BrowserOnly>
      {() => {
        const { Excalidraw, loadFromBlob } =
          require("@excalidraw/excalidraw") as typeof import("@excalidraw/excalidraw");
        const { useState, useEffect } =
          require("react") as typeof import("react");

        function ExcalidrawDiagram({ diagramPath }: DiagramProps) {
          const [excalidrawAPI, setExcalidrawAPI] =
            useState<ExcalidrawImperativeAPI | null>(null);

          useEffect(() => {
            if (excalidrawAPI !== null) {
              fetch(diagramPath)
                .then((res) => res.blob())
                .then((blob) =>
                  loadFromBlob(blob, excalidrawAPI.getAppState(), null)
                )
                .then((data) => {
                  excalidrawAPI.updateScene(data);
                });
            }
          }, [excalidrawAPI]);

          return (
            <div style={{ height: "600px" }}>
              <Excalidraw
                excalidrawAPI={(api) => setExcalidrawAPI(api)}
                isCollaborating={false}
                zenModeEnabled={false}
                viewModeEnabled={true}
                gridModeEnabled={false}
                UIOptions={{
                  canvasActions: {
                    export: false,
                    loadScene: false,
                    saveToActiveFile: false,
                  },
                }}
              />
            </div>
          );
        }

        return <ExcalidrawDiagram diagramPath={diagramPath} />;
      }}
    </BrowserOnly>
  );
}