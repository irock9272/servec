// performance.js - Architecture page (minimal interactions)
document.addEventListener('DOMContentLoaded', function() {
    // Simple hover effects on cards
    const cards = document.querySelectorAll('.principle-card, .security-item, .option');
    cards.forEach(card => {
        card.addEventListener('mouseenter', function() {
            this.style.boxShadow = '0 4px 12px rgba(0, 0, 0, 0.08)';
        });

        card.addEventListener('mouseleave', function() {
            this.style.boxShadow = '';
        });
    });
});