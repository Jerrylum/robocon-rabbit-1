
#include <Tasks.h>
#include <due_can.h>

#define INO_FILE

#include "jen.hpp"

#include "pid.hpp"
#include "bounding_helper.hpp"
#include "rm_motor.hpp"

#define AIR_1 3
#define AIR_2 2

CAN_FRAME tx_msg, rx_msg;

#define GROUP1_MOTOR_COUNT 4
RMM3508Motor group1_rm[GROUP1_MOTOR_COUNT] = {
  RMM3508Motor(0, DIRECT_OUTPUT_MODE),
  RMM3508Motor(1, DIRECT_OUTPUT_MODE),
  RMM3508Motor(2, DIRECT_OUTPUT_MODE),
  RMM3508Motor(3, DIRECT_OUTPUT_MODE)
};

DECLARE_WATCHER(JsonObject, chassis_setting, "rs.s",

  static int count = 0;
  console << "updated pid" << count++ << "\n";
)

DECLARE_WATCHER(JsonArray, gen_output, "rg.o",
  bool air1 = value[0].as<bool>();
  bool air2 = value[1].as<bool>();

  static int count = 0;
  // console << "updated " << count++ << "\n";
  console << count++ << "\n";
)

DECLARE_WATCHER(JsonArray, chassis_output, "rs.o",
  int leftFront = value[0].as<int>();
  int leftRear = value[1].as<int>();
  int rightFront = value[2].as<int>();
  int rightRear = value[3].as<int>();

  group1_rm[0].output = rightRear;
  group1_rm[1].output = leftRear;
  group1_rm[2].output = -rightFront;
  group1_rm[3].output = -leftFront;

  static int count = 0;
  console << "move " << count++ << "\n";
)

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  START_WATCHER(chassis_setting);
  START_WATCHER(gen_output);
  START_WATCHER(chassis_output);

  Can0.begin(CAN_BPS_1000K);  //  For communication with RM motors

  gb.setup(115200);

  Tasks_Add((Task)loop1, 1, 0);
  Tasks_Add((Task)loop2, 10, 0);
  Tasks_Add((Task)loop3, 1, 0);

  // Start task scheduler
  Tasks_Start();
}

void loop1() {  // Serial
  gb.loop();
}

void loop2() {  // Send sensors / encoders data
  // StaticJsonDocument<128> gen_feedback;
  // gen_feedback[0] = 123;
  // gen_feedback[1] = 456;

  // gb.write("rg.f", gen_feedback);

  // StaticJsonDocument<128> shooter_feedback;
  // shooter_feedback[0] = group1_rm[0].unbound_tick;
  // shooter_feedback[1] = group1_rm[0].speed;
  // shooter_feedback[2] = group1_rm[0].output;
  // shooter_feedback[3] = group1_rm[1].unbound_tick;
  // shooter_feedback[4] = group1_rm[1].speed;
  // shooter_feedback[5] = group1_rm[1].output;

  // gb.write("rs.f", shooter_feedback);
}

void loop3() {  // PID Calculation
  tx_msg.id = 0x200;
  tx_msg.length = 8;

  for (int i = 0; i < GROUP1_MOTOR_COUNT; i++) {
    short output = group1_rm[i].get_output();
    tx_msg.data.byte[i * 2] = output >> 8;
    tx_msg.data.byte[i * 2 + 1] = output;
  }

  Can0.sendFrame(tx_msg);
}

// the loop function runs over and over again forever, runs ~14000 times per second
void loop() {
  Can0.watchFor();
  Can0.read(rx_msg);

  for (int i = 0; i < GROUP1_MOTOR_COUNT; i++) {
    if (group1_rm[i].handle_packet(rx_msg.id, rx_msg.data.byte)) break;
  }
}