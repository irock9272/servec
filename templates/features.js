// features.js - Features page interactions
document.addEventListener('DOMContentLoaded', function() {
    // Simple hover effects on feature cards
    const featureCards = document.querySelectorAll('.feature-card');
    
    featureCards.forEach(card => {
        card.addEventListener('mouseenter', function() {
            this.style.boxShadow = '0 4px 12px rgba(0, 0, 0, 0.08)';
        });

        card.addEventListener('mouseleave', function() {
            this.style.boxShadow = '';
        });
    });

    // Code block copy functionality
    const codeBlock = document.querySelector('.code-block');
    if (codeBlock) {
        codeBlock.style.cursor = 'pointer';
        codeBlock.addEventListener('click', function() {
            const code = this.querySelector('code').textContent;
            navigator.clipboard.writeText(code).then(() => {
                this.style.borderColor = 'var(--primary)';
                setTimeout(() => {
                    this.style.borderColor = 'var(--border)';
                }, 300);
            });
        });
    }
});