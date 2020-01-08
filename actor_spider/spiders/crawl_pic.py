# -*- coding: utf-8 -*-
import scrapy
from scrapy_splash import SplashRequest
import logging
# from actor_spider.items import ActorSpiderItem
from urllib import urlretrieve
from actor_spider.settings import IMAGES_STORE
import re
import base64


class CrawlPicSpider(scrapy.Spider):
    name = "001"
    allowed_domains = []
    start_urls = [
        # sheldon lee cooper
        # "https://g.kingyou.wang/search?q=sheldon+lee+cooper&newwindow=1&source=lnms&tbm=isch&sa=X&ved=0ahUKEwj9g-iq5ePXAhWJe7wKHVYRDsEQ_AUICigB&biw=1366&bih=625"
        # jim parsons -- google image
        # "https://g.kingyou.wang/search?newwindow=1&hl=ko&tbm=isch&source=hp&biw=1366&bih=599&ei=SnYfWu3VGovX8QWV6oGYDQ&q=jim+parsons&oq=jim+parsons&gs_l=img.3..0l2j0i30k1l8.1147.3881.0.4198.11.10.0.1.1.0.233.1532.0j1j6.7.0....0...1ac.1.64.img..4.7.1323....0.0ufTbDLEges"

        # Johnny Galecki -- baidu
        # "https://image.baidu.com/search/index?tn=baiduimage&ipn=r&ct=201326592&cl=2&lm=-1&st=-1&fm=result&fr=&sf=1&fmq=1514358615320_R&pv=&ic=0&nc=1&z=&se=1&showtab=0&fb=0&width=&height=&face=0&istype=2&ie=utf-8&word=Johnny+Galecki"
        "https://image.baidu.com/search/index?tn=baiduimage&ie=utf-8&word=Johnny+Galecki"
        # Johnny Galecki -- google image
        # "https://g.kingyou.wang/search?q=Johnny+Galecki&newwindow=1&source=lnms&tbm=isch&sa=X&ved=0ahUKEwiI2PH51anYAhXDoJQKHdq3C9UQ_AUICigB&biw=1366&bih=645"
    ]

    def start_requests(self):
        script = """
        function main(splash)
            local num_scrolls = 100
            local scroll_delay = 1.0
            local scroll_to = splash:jsfunc("window.scrollTo")
            local get_body_height = splash:jsfunc(
                "function() {return document.body.scrollHeight;}"
            )
            assert(splash:go(splash.args.url))
            splash:wait(splash.args.wait)
            local pre_body_height = -1
            local now_body_height
            local count = 0
            for _ = 1, num_scrolls do
                splash:runjs("document.getElementById('smb').click()")
                now_body_height = get_body_height();
                scroll_to(0, now_body_height)
                if (pre_body_height == now_body_height)
                then
                    count = count + 1
                else
                    count = 0
                end
                if (count > 10)
                then
                    break
                end
                pre_body_height = now_body_height
                splash:wait(scroll_delay)
            end
            return splash:html()
        end
        """

        splash_args = {'wait': 1, 'lua_source': script}
        for url in self.start_urls:
            yield SplashRequest(
                url,
                self.parse_baidu_search_result,
                endpoint='execute',
                args=splash_args)

    def parse_baidu_search_result(self, response):
        logging.info(
            u'----------使用splash爬取百度搜索Sheldon Lee Cooper图片-----------')
        img_li_list = response.xpath("//img[@class='main_img img-hover']")
        logging.info(u"find：%d" % len(img_li_list))
        for index, img_li in enumerate(img_li_list):
            url = img_li.xpath("@src").extract_first()
            if url is not None:
                # data:image/jpeg;base64,/.....
                match_group = re.search(r'^data:image/(.+);(.+),(.+)', url,
                                        re.I)
                if match_group is not None:
                    print("Data URL find %d" % index)
                    if (str(match_group.group(2)) != "base64"):
                        print("The encode %s is not known!" %
                              match_group.group(2))
                        continue
                    else:
                        byte_stream = base64.urlsafe_b64decode(
                            str(match_group.group(3)))
                        with open(
                                IMAGES_STORE + "\\" + str(index) + "." +
                                str(match_group.group(1)),
                                mode="wb") as f:
                            f.write(byte_stream)
                else:
                    print(url)
                    urlretrieve(url, IMAGES_STORE + "\\" + str(index) + ".jpg")

    def parse_google_search_result(self, response):
        logging.info(
            u'----------使用splash爬取google搜索Sheldon Lee Cooper图片-----------')
        img_li_list = response.xpath("//img[@class='rg_ic rg_i']")
        logging.info(u"find：%d" % len(img_li_list))
        for index, img_li in enumerate(img_li_list):
            url = img_li.xpath("@src").extract_first()
            if url is not None:
                # data:image/jpeg;base64,/.....
                match_group = re.search(r'^data:image/(.+);(.+),(.+)', url,
                                        re.I)
                if match_group is not None:
                    print("Data URL find %d" % index)
                    if (str(match_group.group(2)) != "base64"):
                        print("The encode %s is not known!" %
                              match_group.group(2))
                        continue
                    else:
                        byte_stream = base64.urlsafe_b64decode(
                            str(match_group.group(3)))
                        with open(
                                IMAGES_STORE + "\\" + str(index) + "." +
                                str(match_group.group(1)),
                                mode="wb") as f:
                            f.write(byte_stream)
                else:
                    print(url)
                    urlretrieve(url, IMAGES_STORE + "\\" + str(index) + ".jpg")
