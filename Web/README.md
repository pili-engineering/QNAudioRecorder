# QNAudioRecorder

## 跑通项目

### 安装依赖

先在本地设备上准备好 Node(14及以上) 和 npm(8及以上) 环境。

```
npm install
```

### 编译

```
npm run build
```

### 测试

```
npm run start
```

打开浏览器，进入 `http://localhost:8080`

## 接口使用

1. 引入 js 文件

上一步 `npm run build` 后会生成 `/dist/bundle.js` 文件，这里引入这个文件。

```html
<script src="./dist/bundle.js"></script>
```

2. 开始录制

```js
// volume 范围 0-1 浮点数
QNAudioRecorder.start(function (volume) {
  console.log(volume);
});
```

3. 结束录制

```js
QNAudioRecorder.stop();
```

> 注意，web 上音频操作需要用户手势介入，即开始录制的方法应当在用户手势回调函数中调用，具体可参考 `./index.html` 文件中的用法演示
