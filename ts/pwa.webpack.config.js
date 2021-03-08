const CopyPlugin = require("copy-webpack-plugin");
const path = require("path");
const webpack = require("webpack");

const dist = path.resolve(__dirname, "../pwa");

module.exports = {
    mode: "development",
    entry: "${CMAKE_CURRENT_BINARY_DIR}/src/programs/clarionpwa/index.ts",
    devtool: "inline-source-map",
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                use: [
                    {
                        loader: "ts-loader",
                        options: {
                            configFile:
                                "${CMAKE_CURRENT_BINARY_DIR}/pwa.tsconfig.json",
                        },
                    },
                ],
                exclude: /node_modules/,
            },
        ],
    },
    plugins: [
        new CopyPlugin({
            patterns: [
                { from: "src/programs/clarionpwa/index.html", to: dist },
                {
                    from: "../wasm/tests/web/a.wasm",
                    to: dist + "/clarion.wasm",
                },
            ],
        }),
        new webpack.EnvironmentPlugin({
            NODE_DEBUG: false,
        }),
    ],
    resolve: {
        extensions: [".tsx", ".ts", ".js"],
        symlinks: false,
    },
    output: {
        filename: "index.js",
        path: dist,
    },
};
