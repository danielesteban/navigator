<script lang="ts">
  import Discord from './components/discord.svelte';
  import Doc from './components/doc.svelte';
  import Navigator from './components/navigator.svelte';
</script>

<Discord />

<div class="main">
  <div class="content">
    <Navigator />
    <Doc />
  </div>
</div>

<style>
  .main {
    width: 100vw;
    height: 100%;
    display: grid;
    justify-items: center;
    overflow-y: overlay;
  }
  .content {
    display: grid;
    grid-template-rows: 100vh auto;
  }
  :global(:root) {
    font-size: 16px;
    width: 100vw;
    height: 100%;
  }
  :global(body) {
    margin: 0;
    background: linear-gradient(to top, #000, #111, #222);
    color: #eee;
    cursor: default;
    user-select: none;
    overflow: hidden;
    font-family: 'Roboto Condensed', monospace;
    font-size: 0.875rem;
    line-height: 1em;
    width: 100vw;
    height: 100%;
    touch-action: none;
    -webkit-touch-callout: none;
    -webkit-text-size-adjust: none;
  }
  :global(a) {
    color: inherit;
    cursor: pointer;
    outline: none;
  }
  :global(::-webkit-scrollbar) {
    background-color: rgba(0, 0, 0, 0);
  }
  :global(::-webkit-scrollbar:hover) {
    background-color: rgba(0, 0, 0, 0);
  }
  :global(::-webkit-scrollbar:vertical) {
    width: 0.5rem;
  }
  :global(::-webkit-scrollbar-thumb:vertical) {
    background: #000;
  }
  :global(::-webkit-scrollbar-thumb:vertical:active) {
    background: #333;
  }
</style>
