<script lang="ts">
  import { onMount } from 'svelte';
  import Github from '../icons/github.svelte';
  import Linux from '../icons/linux.svelte';
  import Windows from '../icons/windows.svelte';

  let canvas: HTMLCanvasElement;
  onMount(() => {
    const { width, height } = canvas.parentElement!.getBoundingClientRect();
    canvas.width = width;
    canvas.height = height;
    const GL: WebGLRenderingContext = canvas.getContext('webgl')!;

    const vertex = GL.createShader(GL.VERTEX_SHADER)!;
    GL.shaderSource(vertex, `
      precision mediump float;

      attribute vec2 position;
      varying vec2 uv;

      void main() {
        uv = (position * 0.5) + 0.5;
        gl_Position = vec4(position, 0.0, 1.0);
      }
    `);
    GL.compileShader(vertex);
    const fragment = GL.createShader(GL.FRAGMENT_SHADER)!;
    GL.shaderSource(fragment, `
      precision mediump float;

      varying vec2 uv;
      uniform float time;

      float sdCircle(in vec2 p, in float r) {
        return length(p) - r;
      }
      float opUnion(in float a, in float b, in float k) {
        float h = clamp(0.5 + 0.5*(b-a)/k, 0.0, 1.0);
        return mix(b, a, h) - k*h*(1.0-h);
      }
      vec2 rotate(in vec2 p, in float r) {
        float c = cos(r);
        float s = sin(r);
        return mat2(c, s, -s, c) * p;
      }
      vec4 sRGB(in vec4 value) {
        return vec4(mix(pow(value.rgb, vec3(0.41666)) * 1.055 - vec3(0.055), value.rgb * 12.92, vec3(lessThanEqual(value.rgb, vec3(0.0031308)))), value.a);
      }
      void main() {
        vec2 p = rotate((uv - 0.5) * vec2(${width / height}, 1.0) * 150.0, time);
        float d = sdCircle(p - vec2(0.0, sin(time) * -20.0), 20.0);
        d = opUnion(d, sdCircle(p - vec2(0.0, sin(time) * 20.0), 20.0), 20.0);
        float a = 1.0 - smoothstep(-0.2, 0.2, d);
        gl_FragColor = vec4(vec3(uv.x, 0.0, uv.y) * a, 1.0);
      }
    `);
    GL.compileShader(fragment);

    const program = GL.createProgram()!;
    GL.attachShader(program, vertex);
    GL.attachShader(program, fragment);
    GL.linkProgram(program);
    GL.useProgram(program);

    const indices = GL.createBuffer()!;
    GL.bindBuffer(GL.ELEMENT_ARRAY_BUFFER, indices);
    GL.bufferData(GL.ELEMENT_ARRAY_BUFFER, new Uint16Array([0, 1, 2, 2, 3, 0]), GL.STATIC_DRAW);
  
    const position = GL.createBuffer()!;
    GL.bindBuffer(GL.ARRAY_BUFFER, position);
    GL.bufferData(GL.ARRAY_BUFFER, new Float32Array([
      -1, -1,
      1, -1,
      1, 1,
      -1, 1,
    ]), GL.STATIC_DRAW);
    GL.vertexAttribPointer(0, 2, GL.FLOAT, false, 0, 0);
    GL.enableVertexAttribArray(0);

    const time = GL.getUniformLocation(program, "time")!;
    const animate = () => {
      requestAnimationFrame(animate);
      GL.uniform1f(time, performance.now() / 1000)
      GL.drawElements(GL.TRIANGLES, 6, GL.UNSIGNED_SHORT, 0);
    };
    let animation = requestAnimationFrame(animate);
    return () => cancelAnimationFrame(animation);
  });

  // @ts-ignore
  const version: string = __VERSION__;
</script>

<div class="navigator">
  <div class="header">
    <img alt="icon" src="/favicon.ico" />
    Homebrew Navigator
  </div>
  <div class="viewport">
    <canvas bind:this={canvas} />
  </div>
  <div class="ui">
    <h1>v{version}</h1>
    <div class="download">
      <a href="https://github.com/danielesteban/navigator/releases/download/v{version}/navigator-v{version}.exe" download>
        <Windows />
        Windows (64bit) - Setup
      </a>
      <a href="https://github.com/danielesteban/navigator/releases/download/v{version}/navigator-v{version}.zip" download>
        <Windows />
        Windows (64bit) - Portable
      </a>
      <a href="https://github.com/danielesteban/navigator/releases/download/v{version}/navigator-v{version}.tar.xz" download>
        <Linux />
        Linux (64bit) - Portable
      </a>
      <a href="https://github.com/danielesteban/navigator" rel="noopener noreferrer" target="_blank">
        <Github />
        Source code
      </a>
    </div>
    <div>
      <a href="https://dani.gatunes.com" rel="noopener noreferrer" target="_blank">dani@gatunes</a> © 2023
    </div>
  </div>
</div>

<style>
  .navigator {
    position: relative;
    width: 800px;
    height: 600px;
    background: #000;
    margin: auto 4rem;
    box-shadow: 0 0 4rem rgba(255, 255, 255, 0.1);
  }
  .header {
    position: absolute;
    top: 0;
    left: 0;
    right: 0;
    padding: 0.5rem;
    background: #000;
    color: #fff;
    display: flex;
    align-items: center;
    gap: 0.5rem;
  }
  .header > img {
    width: 1rem;
    height: 1rem;
  }
  .viewport {
    width: 100%;
    height: 100%;
  }
  .ui {
    position: absolute;
    bottom: 0.5rem;
    left: 0.5rem;
    right: 0.5rem;
    padding: 1rem;
    border-radius: 0.5rem;
    background: rgba(15, 15, 15, 0.94);
    display: flex;
    flex-direction: column;
    gap: 1rem;
    color: #666;
  }
  .ui > h1 {
    margin: 0;
    font-size: 1.25rem;
    font-family: inherit;
    font-weight: inherit;
    line-height: 1em;
    color: #fff;
  }
  .download {
    display: flex;
    gap: 0.5rem;
  }
  .download > a {
    flex-grow: 1;
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 0.5rem;
    color: #fff;
    background: #1a1a1a;
    border-radius: 0.25rem;
    padding: 0.5rem 1rem;
    text-decoration: none;
  }
</style>
