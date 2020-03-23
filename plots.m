subplot(3,1,1)
stem(1:223, ORIGINALMESSAGE, 'g')
grid on
title('Original Message')
xlabel('Index'), ylabel('Value')
xlim([0 223])

subplot(3,1,2)
stem(1:223, CORRUPTEDMESSAGE, 'r')
grid on
title('Corrupted Message')
xlabel('Index'), ylabel('Value')
xlim([0 223])

subplot(3,1,3)
stem(1:223, DECODEDMESSAGE)
grid on
title('Decoded Message')
xlabel('Index'), ylabel('Value')
xlim([0 223])
