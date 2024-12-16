#!/bin/bash

# 函数： bash_message
# 描述： 消息提示函数
# 参数：
#       $1  --  颜色码
#       $2  --  消息
bash_message() {
	echo -e "\033[$1m $2 \033[0m"
}

bash_message 31 天下风云出我辈，一入江湖岁月催。
bash_message 32 皇图霸业谈笑中，不胜人生一场醉。
bash_message 34 提剑跨骑挥鬼蜮，白骨如山鸟惊飞。
bash_message 35 尘世如潮人如水，只叹江湖几人回！

#判断文件夹是否存在 -d
if [[ ! -d "build" ]]; then
	mkdir build 
	cd build
	echo "create build directory"
else
	cd build
	rm -rf *
	echo "clear build directory"
fi

cmake ..

make

