(function () {
  function setupMenu() {
    var toggle = document.querySelector('.menu-toggle');
    var nav = document.getElementById('primary-nav');

    if (!toggle || !nav) {
      return;
    }

    toggle.addEventListener('click', function (event) {
      var isOpen = nav.classList.toggle('open');
      toggle.setAttribute('aria-expanded', String(isOpen));
      toggle.setAttribute('aria-label', isOpen ? 'Close menu' : 'Open menu');
      event.stopPropagation();
    });

    document.addEventListener('click', function (event) {
      var clickedOutside = !nav.contains(event.target) && event.target !== toggle;
      if (clickedOutside && nav.classList.contains('open')) {
        nav.classList.remove('open');
        toggle.setAttribute('aria-expanded', 'false');
        toggle.setAttribute('aria-label', 'Open menu');
      }
    });

    document.addEventListener('keydown', function (event) {
      if (event.key === 'Escape' && nav.classList.contains('open')) {
        nav.classList.remove('open');
        toggle.setAttribute('aria-expanded', 'false');
        toggle.setAttribute('aria-label', 'Open menu');
        toggle.focus();
      }
    });
  }

  function renderFallbackHeader() {
    return [
      '<header class="header">',
      '  <div class="logo"><a href="index.html">Akash Balakrishnan</a></div>',
      '  <nav id="primary-nav" class="nav">',
      '    <a href="index.html">Home</a>',
      '    <a href="projects.html">Projects</a>',
      '    <a href="about.html">About Me</a>',
      '    <a href="CVAI_Resume_260327.pdf" target="_blank" rel="noopener">Resume</a>',
      '  </nav>',
      '</header>'
    ].join('');
  }

  function loadHeader() {
    var host = document.getElementById('header-placeholder');
    if (!host) {
      return;
    }

    fetch('components/header.html')
      .then(function (response) {
        if (!response.ok) {
          throw new Error('Failed to load header component');
        }
        return response.text();
      })
      .then(function (markup) {
        host.innerHTML = markup;
        setupMenu();
      })
      .catch(function (error) {
        host.innerHTML = renderFallbackHeader();
        console.error('Header load error:', error);
      });
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', loadHeader);
  } else {
    loadHeader();
  }
})();
