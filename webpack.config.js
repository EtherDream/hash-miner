export default {
  entry: './src/index.ts',
  devtool: 'source-map',
  module: {
    rules: [
      {
        test: /\.ts$/,
        use: 'ts-loader',
        exclude: /node_modules/,
      },
      {
        test: /\.(js|glsl|wgsl)$/,
        type: 'asset/source',
      },
      {
        test: /\.wasm$/,
        type: 'asset/inline',
        generator: {
          // no `data:` prefix
          dataUrl: content => content.toString('base64')
        }
      },
    ],
    parser: {
      javascript: {
        importMeta: false,
      },
    }
  },
  resolve: {
    extensions: ['.ts'],
  },
  experiments: {
    outputModule: true,
  },
  output: {
    filename: 'index.js',
    path: import.meta.dirname + '/dist',
    library: {
      type: "module",
    },
  },
}