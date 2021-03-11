const TsconfigPathsPlugin = require("tsconfig-paths-webpack-plugin");
const webpack = require("webpack");
// const HtmlWebpackPlugin = require("html-webpack-plugin");
const CopyPlugin = require("copy-webpack-plugin");
const WorkboxPlugin = require("workbox-webpack-plugin");

const path = require("path");
const dist = path.resolve(__dirname, "dist");
const clarionPath = process.env.CLARION_WASM_PATH;

module.exports = {
    entry: "./src/index.ts",
    mode: "development",
    devtool: "inline-source-map",
    devServer: {
        contentBase: "./dist",
        hot: true,
    },
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                use: "ts-loader",
                exclude: /node_modules/,
            },
        ],
    },
    plugins: [
        new CopyPlugin({
            patterns: [
                {
                    from: clarionPath,
                    to: dist + "/clarion.wasm",
                },
                {
                    from: path.resolve(__dirname, "./public"),
                    to: dist + "/",
                },
            ],
        }),
        // new WorkboxPlugin.GenerateSW({
        //     // these options encourage the ServiceWorkers to get in there fast
        //     // and not allow any straggling "old" SWs to hang around
        //     clientsClaim: true,
        //     skipWaiting: true,
        // }),
        // new HtmlWebpackPlugin({
        //     title: "Clarion PWA",
        // }),
        new webpack.EnvironmentPlugin({
            NODE_DEBUG: false,
        }),
    ],
    resolve: {
        extensions: [".tsx", ".ts", ".js"],
        plugins: [new TsconfigPathsPlugin()],
    },
    output: {
        filename: "[name].bundle.js",
        path: dist,
        clean: true,
    },
};
