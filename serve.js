Bun.serve({
  port: 3000,
  async fetch(req) {
    const url = new URL(req.url);
    let path = url.pathname;
    
    if (path === '/') {
      path = '/index.html';
    }

    const file = Bun.file('./web' + path);
    
    if (await file.exists()) {
      return new Response(await file.arrayBuffer(), {
        headers: {
          'Content-Type': getContentType(path),
        },
      });
    }
    
    return new Response('Not found', { status: 404 });
  },
});

console.log('Server running at http://localhost:3000');

function getContentType(path) {
  const ext = path.split('.').pop();
  const types = {
    'html': 'text/html; charset=utf-8',
    'js': 'application/javascript; charset=utf-8',
    'wasm': 'application/wasm',
    'css': 'text/css; charset=utf-8',
    'json': 'application/json; charset=utf-8',
  };
  return types[ext] || 'application/octet-stream';
}
