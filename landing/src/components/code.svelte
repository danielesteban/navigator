<script lang="ts">
  import Highlight from 'svelte-highlight';
  import lua from 'svelte-highlight/languages/lua';

  export let lang: string;
  export let text: string;
</script>

{#if lang == 'lua'}
  <Highlight language={lua} code={text} />
{:else}
  <pre class={lang}><code>{text}</code></pre>
{/if}
