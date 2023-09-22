<script lang="ts">
  export let lang: string;
  export let text: string;
  import Highlight from 'svelte-highlight';
  import lua from 'svelte-highlight/languages/lua';
  import style from 'svelte-highlight/styles/github-dark';
</script>

<svelte:head>
  {@html style}
</svelte:head>

{#if lang == 'lua'}
<Highlight language={lua} code={text} />
{:else}
<pre class={lang}><code>{text}</code></pre>
{/if}
