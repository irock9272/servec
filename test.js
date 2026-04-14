import http from 'k6/http';
import { sleep, check } from 'k6';

export const options = {
  stages: [
    { duration: '10s', target: 100 },
    { duration: '20s', target: 500 },
    { duration: '30s', target: 1000 },
    { duration: '20s', target: 2000 },
    { duration: '10s', target: 0 },
  ],
};

const base = 'http://localhost:6969';

const pages = [
  {
    html: '/index.html',
    assets: ['/styles.css', '/script.js'],
    weight: 5,
  },
  {
    html: '/features.html',
    assets: ['/styles.css', '/features.css', '/script.js', '/features.js'],
    weight: 3,
  },
  {
    html: '/architecture.html',
    assets: ['/styles.css', '/architecture.css', '/script.js', '/architecture.js'],
    weight: 2,
  },
];

function pickPage() {
  const total = pages.reduce((s, p) => s + p.weight, 0);
  let r = Math.random() * total;
  for (const p of pages) {
    r -= p.weight;
    if (r <= 0) return p;
  }
  return pages[0];
}

export default function () {
  const sessionLen = Math.floor(Math.random() * 3) + 2;

  for (let i = 0; i < sessionLen; i++) {
    const page = pickPage();

    const res = http.get(`${base}${page.html}`);
    check(res, { 'page 200': (r) => r.status === 200 });

    for (const asset of page.assets) {
      const a = http.get(`${base}${asset}`);
      check(a, { 'asset 200': (r) => r.status === 200 });
    }

    sleep(Math.random() * 1 + 0.2); // tighter think time = more pressure
  }
}