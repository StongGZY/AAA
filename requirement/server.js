// Usage: node server.js 16540
let http = require("http");
let url = require("url");
let crypto = require("crypto");
let config = require("./config");

// Standard Normal variate using Box-Muller transform.
function GaussianRandom(mean = 0, stdev = 1) {
    let u = 1 - Math.random();
    let v = Math.random();
    let z = Math.sqrt(-2.0 * Math.log(u)) * Math.cos(2.0 * Math.PI * v);
    // Transform to the desired mean and standard deviation:
    return z * stdev + mean;
}

function GenerateMean() {
    return (Math.random() * 2 - 1) * 1e6;
}

// global vars
let mean = GenerateMean();
let generate_mean_time = Date.now() / 1e3;
let last_submit_time = 0;

function sleep(ms) {
    return new Promise((resolve, reject) => {
        setTimeout(resolve, ms);
    });
}

// generate mean every second
(async function () {
    while (true) {
        if (Date.now() / 1e3 - generate_mean_time > 1) {
            mean = GenerateMean();
            generate_mean_time += 1;
        }
        await sleep(1);
    }
})();

const PORT = parseInt(process.argv[2]);
http.createServer(async function (req, res) {
    try {
        let info = url.parse(req.url, true);

        if (info.pathname == "/") {
            let ret = GaussianRandom(mean).toString();
            await sleep(20);
            res.end(ret);
        } else if (info.pathname == "/submit") {
            let now = Date.now() / 1e3;
            if (now - last_submit_time < 0.9) {
                res.end("Submit blocked");
            } else {
                last_submit_time = now;

                let guess = info.query.guess;
                if (!guess || !/^(\-|\+)?\d+(\.\d+)?$/.test(guess)) {
                    res.end(`Wrong param: guess is ${guess}`);
                } else {
                    let guess_value = parseFloat(guess);
                    let message = `time=${parseInt(generate_mean_time)} guess=${guess} error=${Math.abs(guess_value - mean)} port=${PORT}`;
                    let sign = crypto.createHash('sha256').update(config.KEY).update(message).digest('base64');
                    await sleep(200);
                    res.end(message + " sign=" + sign);
                }
            }
        } else {
            res.statusCode = 404;
            res.end("404 Not Found");
        }
    } catch (error) {
        res.statusCode = 500;
        res.end(error.toString());
    }
}).listen(PORT, "0.0.0.0", 100000, () => { console.log(`Listen http://0.0.0.0:${PORT}`) });
