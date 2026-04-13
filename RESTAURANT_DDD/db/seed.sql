INSERT INTO items (id, name, description, price_cents, prep_time_minutes, cooking_method, station, ingredient_cost_cents, supplier_price_cents, is_available) VALUES
    ('item-001', 'Salmon', 'Pan-seared Atlantic salmon with lemon butter sauce', 2800, 12, 'pan-sear', 'grill', 850, 900, 1),
    ('item-002', 'Risotto', 'Wild mushroom risotto with parmesan', 2200, 22, 'simmer', 'sauce', 550, 600, 1),
    ('item-003', 'Lamb', 'Herb-crusted rack of lamb with rosemary jus', 3400, 18, 'roast', 'grill', 1200, 1300, 1),
    ('item-004', 'Caesar Salad', 'Classic Caesar with house-made dressing and croutons', 1400, 5, 'raw', 'cold', 300, 350, 1),
    ('item-005', 'Chocolate Fondant', 'Warm chocolate fondant with vanilla ice cream', 1600, 14, 'bake', 'pastry', 400, 450, 1);

INSERT INTO tables (id, table_number, capacity, status) VALUES
    ('tbl-01', 1, 2, 'available'),
    ('tbl-02', 2, 2, 'available'),
    ('tbl-03', 3, 4, 'available'),
    ('tbl-04', 4, 4, 'available'),
    ('tbl-05', 5, 4, 'available'),
    ('tbl-06', 6, 6, 'available'),
    ('tbl-07', 7, 6, 'available'),
    ('tbl-08', 8, 6, 'available'),
    ('tbl-09', 9, 8, 'available'),
    ('tbl-10', 10, 8, 'available'),
    ('tbl-11', 11, 2, 'available'),
    ('tbl-12', 12, 4, 'available'),
    ('tbl-13', 13, 6, 'available'),
    ('tbl-14', 14, 4, 'available');
