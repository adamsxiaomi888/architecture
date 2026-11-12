# 浏览器 CAD 资产

把官方 STEP 转成 STL 后放在本目录：

```sh
sudo apt install freecad
./tools/convert_step.sh
```

STEP 适合作为工程基准；STL 只用于网页展示和粗略外形查看，不能用于制造公差或安装孔最终确认。STL 已生成并保存。网页可读取 STL 参考文件，但主视图使用程序化外观模型，不是官方 CAD 精确渲染。标签、摇臂和目标提示属于展示层，不代表原装附件或机械装配精度。
