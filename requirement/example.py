import requests
import time

with open("submit.txt", "w") as fout:
    while True:
        v1 = float(requests.get("http://59.110.159.71:16540").text)
        v2 = float(requests.get("http://59.110.159.71:16540").text)
        guess_mean = str((v1+v2)/2)
        print(f"{v1=} {v2=} {guess_mean=}")
        r = requests.get(f"http://59.110.159.71:16540/submit?guess={guess_mean}").text
        print(r)
        fout.write(r + "\n")
        fout.flush()

        time.sleep(1)
