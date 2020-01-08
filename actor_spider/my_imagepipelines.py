# -*- coding: utf-8 -*-

import scrapy
from scrapy.pipelines.images import ImagesPipeline
from scrapy.exceptions import DropItem
import actor_spider.settings
import base64
import re
import random
import logging


class MyImagesPipeline(ImagesPipeline):
    def get_media_requests(self, item, info):
        for image_url, image_title in zip(item['image_urls'],
                                          item['image_titles']):
            if re.search('^data', image_url, re.I) is not None:
                logging.info(image_title + "Data URL")
                file_path = actor_spider.settings.IMAGES_STORE + "\\images_from_dataurl"
                content_start = image_url.rfind(',') + 1
                content = image_url[content_start:]
                file_suffix = image_url[image_url.index('/') + 1,
                                        image_url.index(';')]

                random_part = str(random.randint(1, 50000)) + str(
                    random.randint(50000, 100000)) + str(
                        random.randint(100000, 200000))
                file_name = "\\" + random_part + "." + file_suffix
                with open(file_path + file_name, mode='wb') as f:
                    f.write(base64.urlsafe_b64decode(content))
            else:
                if (re.search('sheldon.*cooper', image_title, re.I) is
                        not None):
                    yield scrapy.Request(image_url)
                else:
                    continue

    def item_completed(self, results, item, info):
        image_paths = [x['path'] for ok, x in results if ok]
        if not image_paths:
            raise DropItem("Item contains no images")
        item['image_paths'] = image_paths
        return item
