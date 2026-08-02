import canopen
import sys


def createOD():
    od = canopen.ObjectDictionary()

    # 添加Array (其包含的子元素类型必须是相同的)
    pgmCtrlArray = canopen.objectdictionary.Array('Program Control', 0x1F51)

    var = canopen.objectdictionary.Variable('Program Number 1', 0x1F51, 1)
    var.data_type = canopen.objectdictionary.UNSIGNED8
    var.access_type = 'rw' # 读写权限
    pgmCtrlArray.add_member(var)
    od.add_object(pgmCtrlArray) # 添加array


    pgmDataArray = canopen.objectdictionary.Array('Program Data', 0x1F50)

    # 添加0x1f50，类型是Domain
    var = canopen.objectdictionary.Variable('Firmware Data', 0x1F50, 1)
    var.data_type = canopen.objectdictionary.DOMAIN  # 设置数据类型为 DOMAIN
    var.access_type = 'rw' # 读写权限
    pgmDataArray.add_member(var)
    od.add_object(pgmDataArray) # 添加array

    return od



nodeid = int(sys.argv[1]) if (len(sys.argv) > 1) else 12


# 1. 初始化网络与节点
network = canopen.Network()
# 假设使用虚拟CAN总线或物理CAN接口进行网络调试
network.connect(bustype='socketcan', channel='vcan0') 

# 加载目标节点的 EDS 文件（对象字典）
node = canopen.RemoteNode(nodeid, createOD())
network.add_node(node)

# 2. 准备要下载的大数据（超过4字节，触发Segment/Block传输）
# 比如要写入一段配置数据或固件片段
data_to_write = b'\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B' * 50 

# 3. 使用文件对象接口进行分段下载

try:
    # 开启 block_transfer=True，库会自动处理分段下载（Segmented Download）协议
    with node.sdo[0x1F50][1].open('wb', size=len(data_to_write), block_transfer=True) as outfile:
        outfile.write(data_to_write)
        
    print("SDO Segment 下载成功！")

except Exception as e:
    print(f"SDO 下载失败: {e}")

finally:
    network.disconnect()