#include <mbed.h>

#include <array>
#include <chrono>

using namespace std;
using namespace std::chrono_literals;
using namespace std::chrono;

namespace {

constexpr int frequency = 50;
constexpr int can_id_base = 140;

constexpr microseconds min_pulse_width = 500us;
constexpr microseconds max_pulse_width = 2500us;
constexpr microseconds default_pulse_width = (max_pulse_width + min_pulse_width) / 2;

}  // namespace

microseconds byte_to_pulse_width(uint8_t b) {
  constexpr int span = max_pulse_width.count() - min_pulse_width.count();
  return microseconds((int32_t)b * span / 255) + min_pulse_width;
}  //CAN指令値に対応するサーボ指令Duty比演算関数

int main() {
  array<PwmOut, 8> pwms{{{PA_1}, {PA_3}, {PA_4}, {PA_6}, {PA_7}, {PA_8}, {PA_9}, {PA_10}}};
  CAN can{PA_11, PA_12, 1000000};
  //PWMとCANの宣言

  // DigitalIn can_id_1(PB_6);
  // DigitalIn can_id_2(PB_7);
  // ID変更スイッチの宣言

  for (auto&& pwm : pwms) {
    pwm.period_us(1'000'000 / frequency);
  }  //すべてのPWM出力ピンのPWMの周期を設定

  for (auto&& pwm : pwms) {
    pwm.pulsewidth_us(default_pulse_width.count());
  }  //すべてのPWM出力ピンの初期動作のパルス幅を設定

  // int can_id = can_id_base + !can_id_2*2 + !can_id_1*1;
  int can_id = can_id_base;

  while (1) {
    CANMessage msg;
    if (can.read(msg) && msg.format == CANFormat::CANStandard && msg.id == can_id && msg.len == pwms.size()) {
      for (size_t i = 0; i < pwms.size(); i++) {
        int pulse_width_us = byte_to_pulse_width(msg.data[i]).count();
        pwms[i].pulsewidth_us(pulse_width_us);
      }
    }
    ThisThread::sleep_for(1ms);
    //1m秒待機
  }
}