/* eslint-disable import/no-extraneous-dependencies */
const TsconfigPathsPlugin = require("tsconfig-paths-webpack-plugin");
const path = require("path");
// const CopyPlugin = require("copy-webpack-plugin"); // todo: fix this...

module.exports = (config) => {
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

    // const clarionPath = process.env.CLARION_WASM_PATH;
    // const assetsFolder = path.resolve(__dirname, "public", "assets");
    // const copyPlugin = new CopyPlugin({
    //     patterns: [
    //         {
    //             from: clarionPath,
    //             to: assetsFolder + "/clarion.wasm",
    //         },
    //         // {
    //         //     from: path.resolve(__dirname, "./public"),
    //         //     to: dist + "/",
    //         // },
    //     ],
    // });

    // if (!config.plugins) {
    //     config.plugins = [];
    // }
    // config.plugins.push(copyPlugin);

    return config;
};
