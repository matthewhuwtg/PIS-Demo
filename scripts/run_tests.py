#!/usr/bin/env python3
"""PIS Test Runner."""
import subprocess, sys, os, argparse
from datetime import datetime

class Runner:
    def __init__(self, proj_dir, verbose=False): self.proj_dir=proj_dir; self.verbose=verbose; self.results=[]
    def run(self, cmd):
        try:
            r = subprocess.run(cmd, cwd=self.proj_dir, capture_output=True, text=True, timeout=120)
            return r.returncode, r.stdout+r.stderr
        except: return -1, "TIMEOUT"
    def build(self):
        print("="*60+"\n  Building tests...\n"+"="*60)
        ret, out = self.run(["make","test"])
        if self.verbose or ret!=0: print(out)
        return ret==0
    def run_tests(self):
        print("="*60+"\n  Executing tests...\n"+"="*60)
        bin_path = os.path.join(self.proj_dir,"bin","test_schedule")
        if not os.path.exists(bin_path): print(f"Binary not found: {bin_path}"); return False
        ret, out = self.run([bin_path])
        if self.verbose or ret!=0: print(out)
        passed = "PASSED" in out or "All tests" in out
        self.results.append({"passed":passed and ret==0,"output":out,"code":ret})
        return passed and ret==0
    def report(self):
        print("="*60+"\n  Report: "+str(datetime.now())+f"\n"+"="*60)
        total=len(self.results); passed=sum(1 for r in self.results if r["passed"])
        for r in self.results: print(f"  [{'PASS' if r['passed'] else 'FAIL'}] exit={r['code']}")
        print(f"\n  {passed}/{total} passed\n"+"="*60)
        return passed==total

def main():
    p = argparse.ArgumentParser(description="PIS Test Runner")
    p.add_argument("--no-build",action="store_true"); p.add_argument("--verbose","-v",action="store_true")
    p.add_argument("--project-dir",default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    args = p.parse_args()
    r = Runner(args.project_dir, args.verbose)
    ok = True
    if not args.no_build: ok = r.build()
    ok = r.run_tests() and ok
    all_ok = r.report()
    sys.exit(0 if all_ok else 1)

if __name__ == "__main__": main()
