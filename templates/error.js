// error.js - Error page interactions
document.addEventListener('DOMContentLoaded', function() {
    // Set current time
    function updateTime() {
        const timeElements = document.querySelectorAll('#error-time');
        const now = new Date();
        const timeString = now.toLocaleString();
        timeElements.forEach(el => el.textContent = timeString);
    }

    updateTime();
    setInterval(updateTime, 1000);

    // Set request path for 404
    const requestPath = document.getElementById('request-path');
    if (requestPath) {
        const url = new URL(window.location.href);
        requestPath.textContent = url.pathname;
    }

    // Generate random error ID for 500
    const errorId = document.getElementById('error-id');
    if (errorId) {
        const timestamp = Date.now().toString(36);
        const random = Math.random().toString(36).substr(2, 5);
        errorId.textContent = `SRV-500-${timestamp}-${random}`.toUpperCase();
    }

    // Add hover effects to buttons
    const buttons = document.querySelectorAll('.error-btn');
    buttons.forEach(button => {
        button.addEventListener('mouseenter', function() {
            this.style.opacity = '0.8';
        });

        button.addEventListener('mouseleave', function() {
            this.style.opacity = '1';
        });
    });

    // Add hover effects to detail items
    const detailItems = document.querySelectorAll('.detail-item');
    detailItems.forEach(item => {
        item.addEventListener('mouseenter', function() {
            this.style.background = 'rgba(0, 0, 0, 0.02)';
            this.style.borderRadius = '4px';
        });

        item.addEventListener('mouseleave', function() {
            this.style.background = 'transparent';
        });
    });
});