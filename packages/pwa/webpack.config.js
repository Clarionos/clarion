const TsconfigPathsPlugin = require("tsconfig-paths-webpack-plugin");
const webpack = require("webpack");
const HtmlWebpackPlugin = require("html-webpack-plugin");
// const CopyPlugin = require("copy-webpack-plugin");

const path = require("path");
const dist = path.resolve(__dirname, "dist");

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
    // new CopyPlugin({
    //   patterns: [
    //     { from: "./public", to: dist },
    //     {
    //       from: "../a.wasm",
    //       to: dist + "/clarion.wasm",
    //     },
    //   ],
    // }),
    new HtmlWebpackPlugin({
      title: "Clarion PWA",
    }),
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
