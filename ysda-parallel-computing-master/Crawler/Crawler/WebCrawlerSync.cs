using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace Crawler
{
    public class WebCrawlerSync : ICrawler
    {
        public WebCrawlerSync(string baseUrl, int maxDepth, int maxPages, string folder)
        {
            this.baseUrl = baseUrl;
            this.maxDepth = maxDepth;
            this.maxPages = maxPages;
            this.folder = folder;
        }

        public void Crawl(bool verbose = false)
        {
            var queue = new Queue<(Link, int)>(new[] {(CrawlHelpers.StripUrl(baseUrl), 0)});
            var visited = new HashSet<string>();
            var downloadedPages = 0;
            while (queue.Count != 0 && downloadedPages < maxPages)
            {
                var (address, currentDepth) = queue.Dequeue();
                if (currentDepth > maxDepth)
                    break;

                if (verbose)
                    Console.WriteLine($"Depth: {currentDepth}/{maxDepth}, " +
                                      $"Downloaded: {downloadedPages}/{maxPages}, " +
                                      $"Downloading: {address}.");

                visited.Add(address.Url);

                if (!CrawlHelpers.TryGetPage(address, out var page))
                    continue;

                downloadedPages++;

                File.WriteAllText($"{folder}/{CrawlHelpers.GetPageFilename(address.Url)}.html", page);

                var links = CrawlHelpers.GetLinks(page)
                    .Where(x => !visited.Contains(x.Url))
                    .ToArray();

                if (verbose)
                    Console.WriteLine($"Adding {links.Length} pages to queue.");

                foreach (var link in links)
                    queue.Enqueue((link, currentDepth + 1));
            }
        }

        private readonly string baseUrl;
        private readonly int maxDepth;
        private readonly int maxPages;
        private readonly string folder;
    }
}