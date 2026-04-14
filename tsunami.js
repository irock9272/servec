import http from 'k6/http';
import { check } from 'k6';

export const options = {
  scenarios: {
    tsunami: {
      executor: 'constant-arrival-rate',
      rate: 50000,        // 50k requests per second
      timeUnit: '1s',
      duration: '10s',
      preAllocatedVUs: 5000,
      maxVUs: 10000,
    },
  },
};

const base = 'http://localhost:6969';
const paths = ['/index.html', '/features.html', '/architecture.html', '/styles.css', '/script.js'];

export default function () {
  const path = paths[Math.floor(Math.random() * paths.length)];
  const res = http.get(`${base}${path}`);
  check(res, { 'ok': (r) => r.status === 200 });
}