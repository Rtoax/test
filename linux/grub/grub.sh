#!/bin/bash
# 修改 启动的内核
# 荣涛 2021年2月22日

function __rt_centos7_grub_kernel_select {

# root
	if [ $UID != 0 ]; then
		echo "must be root."
		exit
	fi

	cat /etc/redhat-release | grep "CentOS Linux" | grep "7" 2>&1 > /dev/null
	if [ $? -ne 0 ]; then
		echo "\"该配置脚本只支持 CentOS Linux 7\""
		exit 1
	fi

# 当前内核
	echo "[grub]"
	curr_kernel=`grub2-editenv list`
	curr_kernel=${curr_kernel#*=}

# 查看操作系统与可用的内核
	echo "[grub]"
	cat /etc/redhat-release | sed 's/^/[grub]当前系统版本: &/g' 
	echo $curr_kernel		| sed 's/^/[grub]当前内核版本: &/g'

# 获取内核列表
	echo "[grub]"
	kernel_list=$(sudo cat /boot/grub2/grub.cfg |grep menuentry | grep Core | awk '{print $4}' | sed 's/[(|)]//g')
	nkernel=$(echo $kernel_list | wc -w)

# 打印内核列表
	i=1
	echo "[grub]可用的内核(当前内核 \"`uname -r`\" ，共 $nkernel 个)："
	for ikernel in $kernel_list
	do
		echo "[grub]	第 $i 个: \"CentOS Linux ($ikernel) 7 (Core)\""
		i=$(expr $i + 1 )
	done

# 配置内核
	echo "[grub]"
	while read -r -p "[grub] 输入需要使用配置的内核[例: \"3.10.0-1062.el7.x86_64\"]: " selected_kernel
	do
		case $selected_kernel in
			*)
			if [ ! -z $selected_kernel ]; then

				# 如果当前内核即为 选择的内核
				echo $kernel_list | grep $selected_kernel 2>&1 > /dev/null
				if [ $? == 0 ]; then
				
					# 如果当前内核即为 选择的内核
					if [ "$curr_kernel" == "CentOS Linux ($selected_kernel) 7 (Core)" ]; then
						echo "[grub] 当前内核即为 \"CentOS Linux ($selected_kernel) 7 (Core)\""

					# 如果当前内核 不是 选择的内核，进行配置
					else
						grub2-set-default "CentOS Linux ($selected_kernel) 7 (Core)"
						echo "[grub] 配置内核为 \"CentOS Linux ($selected_kernel) 7 (Core)\""
					fi
					break
				fi
			fi
			echo "[grub] 错误的输入！"
			;;
		esac
	done

# 查询系统安装了那些内核包
	echo "[grub] 系统安装的内核包："
	rpm -qa | grep kernel | sed 's/^/[grub]   >>&/g' 
	echo "[grub] 使用yum remove 或rpm -e 删除无用内核"
	echo "[grub] 	例如：yum remove kernel-3.10.0-327.el7.x86_64"
}

#!/bin/bash
function __rt_centos7_grub_kernel_select2 {

#source "term-colors"

	DISTRIBUTIVE=$(cat /etc/*-release | grep NAME | head -1 | sed -n -e 's/NAME\=//p')
	echo -e "Distributive: ${Green}${DISTRIBUTIVE}${Color_Off}"

	if [[ "$DISTRIBUTIVE" == "Fedora" ]] ;
	then
	    su -c 'grub2-mkconfig -o /boot/grub2/grub.cfg'
	else
	    sudo update-grub
	fi

	echo "${Green}Done.${Color_Off}"
}

__rt_centos7_grub_kernel_select
#__rt_centos7_grub_kernel_select2
