document.addEventListener('DOMContentLoaded', function() {
    const subpage = window.location.pathname;

    switch (subpage) {
        case "/firstpair":
            var btn = document.getElementById('firstpair_button');
            btn.classList.remove('button-off');
            btn.classList.add("button-on");
            break;
        case "/secondpair":
            var btn = document.getElementById('secondpair_button');
            btn.classList.remove('button-off');
            btn.classList.add("button-on");            
            break;
        case "/alloff":
            var btn = document.getElementById('alloff_button');
            btn.classList.remove('button-off');
            btn.classList.add("button-on");
            break;
        default:
            break;
    }
});

const buttons = document.querySelectorAll('.button-off');

buttons.forEach(button => {
    button.addEventListener('click', function(event) {
        event.preventDefault();
        buttons.forEach(btn => btn.classList.remove('button-off'));
        this.classList.add('button-off');

        const href = this.parentElement.getAttribute('href');
        window.location.href = href;
    });
});