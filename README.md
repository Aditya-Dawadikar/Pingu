# 🐧 Pingu - REST API Testing CLI

**Pingu** is a lightweight and developer-friendly CLI tool to test REST APIs and validate JSON responses with detailed diff output and support for parallel execution.

---

## 🔧 Features

- Run individual test cases or entire test suites
- JSON diff with compact or structured output
- Ignore specific fields during comparison
- Parallel test execution
- Export test logs as JSON
- Simple HTTP ping check (`--ping`)
- CLI-friendly output with colored diffs and timing info

---

## 🚀 Usage


### Run for help

    pingu --help

![img](https://github.com/Aditya-Dawadikar/Pingu/blob/master/views/help.png)

### Run a single test case

    pingu --test test_spec.json

![img](https://github.com/Aditya-Dawadikar/Pingu/blob/master/views/single_test_out.png)

### Run a full test suite

    pingu --test_suit test_suite.json

![img](https://github.com/Aditya-Dawadikar/Pingu/blob/master/views/test_suit_out.png)

### Run tests in parallel (only for test suites)

    pingu --test_suit suite.json --parallel

![img](https://github.com/Aditya-Dawadikar/Pingu/blob/master/views/test_suit_out_parallel.png)

### Compact diff output

    pingu --test test_spec.json --compact

![img](https://github.com/Aditya-Dawadikar/Pingu/blob/master/views/test_suit_out_compact.png)

### Export logs

    pingu --test_suit suite.json --export-log results.json

![img](https://github.com/Aditya-Dawadikar/Pingu/blob/master/views/exports.png)

### Ping a URL

    pingu --ping https://example.com

You can also customize ping behavior:

    pingu --ping https://example.com --ping-timeout 5000 --ping-retries 3

---

## 📄 Test Spec Format

A single test spec (`test_spec.json`):

```json
{
  "test_name": "Simple POST test",
  "test_description": "Sends JSON and validates response",
  "request_description": "request.json",
  "expected_response": "response.json",
  "ignore": ["headers.X-Amzn-Trace-Id", "timestamp"]
}
```


A test suite (`test_suite.json`):
```json
{
  "test_suit_name": "Sample Suite",
  "test_suit_description": "Validates multiple endpoints",
  "test_cases": [
    {
      "test_name": "Ping",
      "request_description": "ping_request.json",
      "expected_response": "ping_response.json"
    }
  ]
}
```

📦 Dependencies
- C++17
- nlohmann/json
- libcurl (for HTTP requests)
- CMake (for building)
