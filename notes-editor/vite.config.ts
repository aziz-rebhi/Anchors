import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

// Embedded into a QtWebEngineView, so every asset URL has to be relative to
// the document. Without this, the dev build emits /assets/... which resolves
// against the WebEngine origin rather than the bundle's directory.
export default defineConfig({
  base: "./",
  plugins: [react()],
  server: {
    port: 5173,
    // Fail loudly instead of silently hopping to 5174 - the Qt shell hardcodes
    // http://localhost:5173/ and would otherwise show a blank editor.
    strictPort: true,
  },
  build: {
    outDir: "dist",
    assetsDir: "assets",
    emptyOutDir: true,
    target: "es2022",
  },
});