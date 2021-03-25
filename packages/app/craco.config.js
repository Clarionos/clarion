const TsconfigPathsPlugin = require("tsconfig-paths-webpack-plugin");
const { ESLINT_MODES } = require("@craco/craco");
const path = require("path");
const CopyPlugin = require("copy-webpack-plugin");

module.exports = {
    style: {
        postcss: {
            plugins: [require("tailwindcss"), require("autoprefixer")],
        },
    },
    eslint: {
        mode: ESLINT_MODES.extends,
        configure: require("../../.eslintrc"),
    },
    webpack: {
        configure: (config) => {
            // Remove the ModuleScopePlugin which throws when we try to import something
            // outside of src/.
            config.resolve.plugins.pop();

            // Resolve the path aliases.
            config.resolve.plugins.push(new TsconfigPathsPlugin());

            // Let Babel compile outside of src/.
            const oneOfRule = config.module.rules.find((rule) => rule.oneOf);
            const tsRule = oneOfRule.oneOf.find((rule) =>
                rule.test.toString().includes("ts|tsx")
            );
            tsRule.include = undefined;
            tsRule.exclude = /node_modules/;

            const clarionPath = process.env.CLARION_WASM_PATH;
            const assetsFolder = path.resolve(__dirname, "public", "assets");
            console.info("watching and copying wasm to ", assetsFolder);
            const copyPlugin = new CopyPlugin({
                patterns: [
                    {
                        from: clarionPath,
                        to: assetsFolder + "/clarion.wasm",
                    },
                ],
            });

            if (!config.plugins) {
                config.plugins = [];
            }
            config.plugins.push(copyPlugin);

            return config;
        },
    },
};
