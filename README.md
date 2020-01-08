## Image Spider

[![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](LICENSE)

### Description
Crawling images in a specific HTML page based on the [Scrapy][] framework. Obtained data is mainly used for a face identification project. This spider can be futher improved to crawl images from any modern search engines.

Two modules:
* Spider (actor_spider) based on [Scrapy][]
* Filter (data_postprocess) based on [SeetaFace][]

### Limitations
* Modern web pages embrace dynamic loading,  thus the spider may needs some Lua scripts to obtain the whole page.
* Too low the abstract level is.

[Scrapy]: https://github.com/scrapy/scrapy
[SeetaFace]: https://github.com/seetaface/SeetaFaceEngine