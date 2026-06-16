#!/usr/bin/env python3
"""PIS Configuration Generator."""
import json, argparse, sys, os

def generate(num_stations=7, route_name="Line 1", ipc=True, dbus=True, audio=True):
    templates = [("Central","中央"),("Market","市场"),("University","大学"),("Tech Park","科技园"),("Sports Center","体育"),("Railway","火车"),("Airport","机场")]
    stations = []
    for i in range(num_stations):
        en, zh = templates[i % len(templates)]
        stations.append({"id":f"S{i+1:02d}","name_en":f"{en} Station","name_zh":f"{zh}站","arrival_sec":i*120+(30 if i>0 else 0)})
    return {"system":{"name":f"PIS - {route_name}","version":"1.0.0","language":"bilingual","languages":["en","zh"]},"route":{"id":"L1","name":route_name,"direction":"clockwise","stations":stations},"display":{"refresh_interval_ms":1000,"color_scheme":"dark","show_next_stop":True,"show_time":True},"ipc":{"socket_path":"/tmp/pis_demo.sock","enabled":ipc},"dbus":{"service_name":"com.pis.demo","object_path":"/com/pis/demo","interface_name":"com.pis.demo.Interface","enabled":dbus},"media":{"announcement_enabled":audio,"audio_backend":"gstreamer"}}

def main():
    p = argparse.ArgumentParser(description="PIS Config Generator")
    p.add_argument("-o","--output",default=""); p.add_argument("-n","--stations",type=int,default=7); p.add_argument("-r","--route",default="Line 1")
    p.add_argument("--no-ipc",action="store_false",dest="ipc"); p.add_argument("--no-dbus",action="store_false",dest="dbus"); p.add_argument("--no-audio",action="store_false",dest="audio"); p.add_argument("--minify",action="store_true")
    args = p.parse_args()
    config = generate(args.stations, args.route, args.ipc, args.dbus, args.audio)
    output = json.dumps(config, ensure_ascii=False, indent=None if args.minify else 2)
    if args.output:
        with open(args.output,"w",encoding="utf-8") as f: f.write(output)
        print(f"[OK] Config written: {args.output}")
    else: print(output)

if __name__ == "__main__": main()
