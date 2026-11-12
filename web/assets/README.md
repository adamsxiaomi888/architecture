# 浏览器 CAD 资产

把官方 STEP 转成 STL 后放在本目录：

```sh
sudo apt install freecad
./tools/convert_step.sh
```

STEP 适合作为工程基准；STL 只用于网页展示和粗略外形查看，不能用于制造公差或安装孔最终确认。网页当前仍保留无依赖 3D 几何后备模型，待资产生成后再接入 STL/GLB 加载器。
