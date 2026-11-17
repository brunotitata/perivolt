import { test } from 'node:test';
import assert from 'node:assert/strict';
import { filterTasks, emptyFilters, sourceIsStale, safeExternalUrl } from './dashboard-model.ts';

test('combines all filters and preserves unknown status without inferring owner activity', () => {
  const tasks = [
    { task_code: 'UI01', title: 'Interface local', status_normalized: 'unknown', owner_code: 'UI', phase: 2, area: 'Frontend' },
    { task_code: 'UI02', title: 'Interface antiga', status_normalized: 'done', owner_code: 'UI', phase: 1, area: 'Frontend' },
  ];
  const result = filterTasks(tasks, { ...emptyFilters, query: 'interface', status: 'unknown', owner: 'UI', phase: '2', area: 'Frontend' });
  assert.deepEqual(result.map(task => task.task_code), ['UI01']);
  assert.equal(tasks[0].status_normalized, 'unknown');
});
test('source ages even while API is unavailable', () => {
  assert.equal(sourceIsStale({ stale: false, age_seconds: 9, stale_after_seconds: 10 }, 2), true);
  assert.equal(sourceIsStale({ stale: true, age_seconds: 0, stale_after_seconds: 10 }, 0), true);
  assert.equal(sourceIsStale({ stale: false, age_seconds: 1, stale_after_seconds: 10 }, 0), false);
});
test('external evidence links accept only absolute http and https', () => {
  for (const bad of ['javascript:alert(1)', 'file:///private', '//example.com', '/api/private', null]) assert.equal(safeExternalUrl(bad), null);
  assert.equal(safeExternalUrl('https://example.com/evidence'), 'https://example.com/evidence');
});
