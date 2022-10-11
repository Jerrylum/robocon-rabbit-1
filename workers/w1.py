from jen import *


def run(worker: WorkerController):
    worker.init()
    worker.use_clock(frequency=100)
    sm = worker.use_serial_manager()
    sm.whitelist.append(PortInfo(serial_number="55131323737351601271", baudrate=115200))
    sm.whitelist.append(PortInfo(serial_number="0001", baudrate=921600))

    gb.start_gateway(UDPBroadcast("255.255.255.255", 7986))

    drive = [0, 0, 0, 0]

    while True:

        drive[0] = 1200
        drive[1] = 1200
        drive[2] = 1200
        drive[3] = 1200
        gb.write("rs.o", list(drive))

        # print(drive)
        # print("feedback", gb.read("feedback"))

        worker.spin()
