from serial import Serial
import time

import cmd


class BqCli(cmd.Cmd):
    intro = 'Welcome to BQ CLI.\n'
    prompt = '>> '
    file = None

    def __init__(self):
        super().__init__()

        self.port = ""
        self.baudrate = 9600
        self.timeout = 0.1

    def preloop(self):
        port = input("Port: ")

        try:
            self.port = port
            self.build_serial_com()
        except:
            print("Not possible to open port")
            exit(0)

    def do_read_eepron(self, args):
        """
        Args: addr
        """
        try:
            addr, = self.parse(args)
            res = self.write_read(f'R{addr}#')
            print("   ", res.decode("utf-8"))
        except:
            print("Invalid syntax")

    def do_write_eepron(self, args):
        """
        Args: addr, data 
        """
        try:
            addr, data, = self.parse(args)
            res = self.write_read(f'W{addr}#{data}#')
            print("   ", res.decode("utf-8"))
        except:
            print("Invalid syntax")

    def do_set_port(self, arg):
        self.port = arg
        self.build_serial_com()

    def do_baud_rate(self, arg):
        self.port = int(arg)
        self.build_serial_com()

    def do_timeout(self, arg):
        self.port = float(arg)
        self.build_serial_com()

    def do_serial_number(self, arg):
        res = self.write_read('a')
        print("   ", res.decode("utf-8"))

    def do_manufacturer_name(self, arg):
        res = self.write_read('b')
        print("   ", res.decode("utf-8"))

    def do_device_name(self, arg):
        res = self.write_read('c')
        print("   ", res.decode("utf-8"))

    def do_device_chemistry(self, arg):
        res = self.write_read('d')
        print("   ", res.decode("utf-8"))

    def do_manufacturer_data(self, arg):
        res = self.write_read('e')
        print("   ", res.decode("utf-8"))

    def do_remaning_capacity_alarm(self, arg):
        res = self.write_read('f')
        print("   ", res.decode("utf-8"))

    def do_design_capacity(self, arg):
        res = self.write_read('g')
        print("   ", res.decode("utf-8"))

    def do_full_charge_capacity(self, arg):
        res = self.write_read('h')
        print("   ", res.decode("utf-8"))

    def do_manufacture_date(self, arg):
        res = self.write_read('i')
        print("   ", res.decode("utf-8"))

    def do_design_voltage(self, arg):
        res = self.write_read('j')
        print("   ", res.decode("utf-8"))

    def do_end_of_discharge_voltage_1(self, arg):
        res = self.write_read('k')
        print("   ", res.decode("utf-8"))

    def do_end_of_discharge_voltage_final(self, arg):
        res = self.write_read('l')
        print("   ", res.decode("utf-8"))

    def do_battery_mode(self, arg):
        res = self.write_read('m')
        print("   ", res.decode("utf-8"))

    def do_battery_status(self, arg):
        res = self.write_read('n')
        print("   ", res.decode("utf-8"))

    def do_charging_current(self, arg):
        res = self.write_read('p')
        print("   ", res.decode("utf-8"))

    def do_charging_voltage(self, arg):
        res = self.write_read('q')
        print("   ", res.decode("utf-8"))

    def do_cycle_count(self, arg):
        res = self.write_read('r')
        print("   ", res.decode("utf-8"))

    def do_flag1_and_flag2(self, arg):
        res = self.write_read('s')
        print("   ", res.decode("utf-8"))

    def do_remaining_capacity(self, arg):
        res = self.write_read('t')
        print("   ", res.decode("utf-8"))

    def do_relative_state_of_charge(self, arg):
        res = self.write_read('u')
        print("   ", res.decode("utf-8"))

    def do_absolute_state_of_charge(self, arg):
        res = self.write_read('v')
        print("   ", res.decode("utf-8"))

    def do_average_time_to_full(self, arg):
        res = self.write_read('w')
        print("   ", res.decode("utf-8"))

    def do_average_time_to_empty(self, arg):
        res = self.write_read('x')
        print("   ", res.decode("utf-8"))

    def do_voltage(self, arg):
        res = self.write_read('y')
        print("   ", res.decode("utf-8"))

    def do_manufacturer_access(self, arg):
        res = self.write_read('z')
        print("   ", res.decode("utf-8"))

    def do_at_rate(self, arg):
        res = self.write_read('A')
        print("   ", res.decode("utf-8"))

    def do_at_rate_time_to_full(self, arg):
        res = self.write_read('B')
        print("   ", res.decode("utf-8"))

    def do_at_rate_time_to_empty(self, arg):
        res = self.write_read('C')
        print("   ", res.decode("utf-8"))

    def do_at_rate_ok(self, arg):
        res = self.write_read('b')
        print("   ", res.decode("utf-8"))

    def do_temperature(self, arg):
        res = self.write_read('b')
        print("   ", res.decode("utf-8"))

    def do_current(self, arg):
        res = self.write_read('b')
        print("   ", res.decode("utf-8"))

    def do_average_current(self, arg):
        res = self.write_read('D')
        print("   ", res.decode("utf-8"))

    def parse(self, arg):
        'Convert a series of zero or more numbers to an argument tuple'
        return tuple(map(int, arg.split()))

    def build_serial_com(self):
        self.serial = Serial(
            port=self.port, baudrate=self.baudrate, timeout=self.timeout)

    def write_read(self, message: str) -> bytes:
        self.serial.write(bytes(message, 'utf-8'))
        time.sleep(0.05)
        data = self.serial.readline()
        return data


if __name__ == "__main__":
    BqCli().cmdloop()
