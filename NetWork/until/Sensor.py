class ABSENC:
    def __init__(self):
        self.modecmd1 = bytes([0x01, 0x06, 0x00, 0x06, 0x00, 0x01, 0xA8, 0x0B])              # 自动回传
        self.modecmd2 = bytes([0x01, 0x06, 0x00, 0x06, 0x00, 0x00, 0x69, 0xCB])              # 查询
        self.modecmd3 = bytes([0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B])              # 读取编码器虚拟多圈值
        self.modecmd4 = bytes([0x01, 0x06, 0x00, 0x07, 0x00, 0x64, 0xF8, 0x08])              # 设置编码器自动回传时间（5ms，默认20ms）
        self.modecmd5 = bytes([0x01, 0x06, 0x00, 0x08, 0x00, 0x01, 0xC9, 0xC8])              # 设置编码器零点（当前点）
        self.modecmd6 = bytes([0x01, 0x06, 0x00, 0x09, 0x00, 0x01, 0x98, 0x08])              # 设置编码器数值递增方向（逆时针数值增加）
        self.BUF_SIZE = 10
        self.buf = bytearray([0x01, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
        self.velocity_pre = self.velocity_c = 0
        self.angle_pre = self.angle_c = 0
        self.angle_total_c = self.angle_total_pre = 0
        self.counter = 0

    def CRC16(self, data, length):
        CRC = 0xFFFF
        for i in range(length):
            CRC ^= data[i]
            for _ in range(8):
                if CRC & 0x0001:
                    CRC = (CRC >> 1) ^ 0xA001
                else:
                    CRC >>= 1
        return CRC

    def angle(self, a_c):
        self.angle_c = (a_c / 16384) * 360

    def angle_total(self):
        if self.angle_c - self.angle_pre < -180:
            self.counter += 1
        if self.angle_c - self.angle_pre > 180:
            self.counter -= 1
        self.angle_total_c = self.angle_c + 360 * self.counter

    def velocity(self, a_c, a_pre):
        self.velocity_c = (a_c - a_pre) / 0.005