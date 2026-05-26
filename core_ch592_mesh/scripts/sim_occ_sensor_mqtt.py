#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
临时脚本：模拟有人无人传感器，周期性向 MQTT 发布 JSON 假数据（含四路 ADC、有人无人）。

使用前请先安装：
    pip install paho-mqtt

示例：
    python sim_occ_sensor_mqtt.py --host 127.0.0.1 --topic sensor/demo --interval 3
"""

from __future__ import annotations

import argparse
import json
import random
import sys
import time


def build_fake_payload(seq: int) -> dict:
    """构造假的 ADC（uint16）并按阈值推断 occupied（≥2 路 >2000 为有人）。"""
    threshold = 2000
    min_over = 2
    adc = [random.randint(1500, 4095) for _ in range(4)]
    over = sum(1 for x in adc if x > threshold)
    occupied = over >= min_over
    return {
        "seq": seq,
        "adc": adc,
        "occupied": occupied,
        "occ_byte": 1 if occupied else 0,
        "threshold": threshold,
        "source": "sim_occ_sensor_mqtt.py",
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="模拟传感器 MQTT 假数据发布")
    parser.add_argument("--host", default="127.0.0.1", help="MQTT broker 地址")
    parser.add_argument("--port", type=int, default=1883, help="MQTT 端口")
    parser.add_argument(
        "--topic",
        default="sensor/occupancy/fake",
        help="发布主题（可自行改成网关订阅的主题）",
    )
    parser.add_argument("--interval", type=float, default=3.0, help="发布间隔（秒）")
    parser.add_argument("--count", type=int, default=0, help="共发送多少次（0=无限循环）")
    parser.add_argument("--username", default="", help="MQTT 用户名（可选）")
    parser.add_argument("--password", default="", help="MQTT 密码（可选）")
    args = parser.parse_args()

    try:
        import paho.mqtt.client as mqtt
    except ImportError:
        print("请先安装：pip install paho-mqtt", file=sys.stderr)
        return 1

    seq = 0

    def on_connect(client, userdata, flags, rc, properties=None):
        print(f"[MQTT] on_connect: {rc}")

    cid = f"sim_occ_{random.randint(1000, 9999)}"
    client = mqtt.Client(client_id=cid)
    client.on_connect = on_connect
    if args.username:
        client.username_pw_set(args.username, args.password or None)

    client.connect(args.host, args.port, keepalive=60)
    client.loop_start()

    sent = 0
    try:
        while True:
            seq += 1
            payload = build_fake_payload(seq)
            body = json.dumps(payload, ensure_ascii=False)
            inf = client.publish(args.topic, body, qos=0)
            inf.wait_for_publish(timeout=5)
            occ_str = "有人" if payload["occupied"] else "无人"
            print(f"[{seq}] {occ_str} adc={payload['adc']} -> {args.topic}")

            sent += 1
            if args.count and sent >= args.count:
                break
            time.sleep(args.interval)
    except KeyboardInterrupt:
        print("\n已停止。")
    finally:
        client.loop_stop()
        client.disconnect()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
