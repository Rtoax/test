#!/bin/bash

# Frequency count stack traces that lead to the submit_bio() function (disk I/O issue):
stackcount submit_bio

# Count stack traces that led to issuing block I/O, tracing its kernel tracepoint:
stackcount t:block:block_rq_insert
