<script lang="ts">
  import SvelteMarkdown from 'svelte-markdown';
  import code from './code.svelte';
  import source from '../../../doc.md';

  const renderers = { code };
</script>

<div class="doc">
  <SvelteMarkdown {renderers} {source} />
</div>
<style>
  .doc {
    padding: 1rem;
    color: #aaa;
    line-height: 1.2em;
    user-select: text;
  }
  .doc :global(h1) {
    font-family: inherit;
    color: #eee;
  }
  .doc :global(h5 > code), .doc :global(li > code) {
    color: #ddd;
    font-family: inherit;
    font-weight: 600;
  }
  .doc :global(pre > code) {
    border-radius: 0.5rem;
  }
  .doc :global(h1) {
    margin: 3rem 0 1.75rem;
  }
  .doc :global(h5) {
    font-family: inherit;
    font-size: inherit;
    font-weight: inherit;
    line-height: inherit;
    margin: 0.5rem 0 0;
  }
  .doc :global(a) {
    color: rgb(26, 128, 255);
  }
  .doc :global(h1):first-child, .doc :global(h5):first-child {
    margin-top: 0;
  }
  .doc :global(ul) {
    margin: 0.5rem 0 1rem;
    padding-inline-start: 1rem;
  }
</style>
