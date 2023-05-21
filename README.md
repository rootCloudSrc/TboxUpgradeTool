
# 20230521首次提交
功能：
	1. 支持ssh连接（测试连接ssh代理服务器18.196.0.17 ，盒子都能连接成功）
	2. 支持sftp读取服务器指定目录和文件，但测试盒子无效（可能盒子需要scp协议）
	3. 支持sftp上传文件到服务器指路径



# TboxUpgradeTool
pro盒子连接wifi，利用本地ssh升级ota包（实际就是ssh上传文件）
开发环境：
	Qt creator版本5.14.1
	编译工具链 MSVC 2017 32bit
	依赖库libssh2-1.10.0


