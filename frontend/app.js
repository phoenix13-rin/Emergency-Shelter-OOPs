// This file only displays data and forwards form inputs to the C++ backend.
const element = id => document.getElementById(id);
let state;

function escapeHTML(text) {
    return String(text).replace(/[&<>"']/g, character => ({
        '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
    }[character]));
}

function showMessage(text, error = false) {
    element('message').textContent = text;
    element('message').className = error ? 'error' : '';
}

async function request(path, fields) {
    const response = await fetch('/api/' + path, {
        method: fields ? 'POST' : 'GET',
        headers: fields ? {'Content-Type': 'application/x-www-form-urlencoded'} : {},
        body: fields ? new URLSearchParams(fields) : undefined
    });
    const data = await response.json();
    if (!response.ok) throw new Error(data.error || 'Request failed.');
    return data;
}

function render() {
    element('people').textContent = state.occupancy;
    element('spaces').textContent = state.capacity - state.occupancy;
    element('count').textContent = state.allocations.length;

    element('resources').innerHTML = state.resources.map(resource => `
        <tr>
            <td>${resource.id}</td><td>${escapeHTML(resource.name)}</td>
            <td>${resource.type}</td><td>${resource.quantity} ${resource.unit}</td>
            <td>${resource.expiry || 'Not applicable'}</td>
            <td class="${resource.usable ? '' : 'blocked'}">${resource.usable ? 'Usable' : 'Expired / empty'}</td>
        </tr>`).join('');

    element('residents').innerHTML = state.residents.map(person => `
        <tr><td>${person.id}</td><td>${escapeHTML(person.name)}</td><td>${person.members}</td></tr>
    `).join('');

    const resourceOptions = state.resources.map(resource => `
        <option value="${resource.id}">${resource.id}: ${escapeHTML(resource.name)} (${resource.quantity} ${resource.unit})</option>
    `).join('');
    element('resource-select').innerHTML = resourceOptions;
    element('restock-select').innerHTML = resourceOptions;
    element('resident-select').innerHTML = state.residents.map(person => `
        <option value="${person.id}">${person.id}: ${escapeHTML(person.name)}</option>
    `).join('');

    element('allocations').innerHTML = state.allocations.map(record => {
        const person = state.residents.find(person => person.id === record.residentId);
        const resource = state.resources.find(resource => resource.id === record.resourceId);
        return `<tr><td>${escapeHTML(person.name)}</td><td>${escapeHTML(resource.name)}</td><td>${record.quantity} ${resource.unit}</td></tr>`;
    }).join('') || '<tr><td colspan="3">No allocations yet.</td></tr>';

    element('restocks').innerHTML = state.restocks.map(record => {
        const resource = state.resources.find(resource => resource.id === record.resourceId);
        return `<tr><td>${escapeHTML(resource.name)}</td><td>${record.quantity} ${resource.unit}</td><td>${escapeHTML(record.donor || 'Ordinary restock')}</td></tr>`;
    }).join('') || '<tr><td colspan="3">No restocks yet.</td></tr>';
}

function connectForm(formId, endpoint) {
    element(formId).addEventListener('submit', async event => {
        event.preventDefault();
        const form = event.target;
        const button = form.querySelector('button');
        button.disabled = true;
        try {
            const fields = Object.fromEntries(new FormData(form));
            if (fields.name) fields.name = fields.name.trim();
            if (fields.donor) fields.donor = fields.donor.trim();
            state = await request(endpoint, fields);
            render();
            let message = 'Success. C++ accepted the operation.';
            if (endpoint === 'restock') {
                message += fields.donor ? ' Used addStock(amount, donor).' : ' Used addStock(amount).';
            }
            showMessage(message);
        }
        catch (error) {
            showMessage('Operation rejected: ' + error.message, true);
        }
        finally {
            button.disabled = false;
        }
    });
}

connectForm('resource-form', 'resources');
connectForm('resident-form', 'residents');
connectForm('allocation-form', 'allocations');
connectForm('restock-form', 'restock');

element('type').addEventListener('change', () => {
    const foodSelected = element('type').value === 'Food';
    element('expiry-label').hidden = !foodSelected;
    element('expiry').required = foodSelected;
});

element('reset').addEventListener('click', async () => {
    if (!confirm('Reset all households, resources and history to the example data?')) return;
    try {
        state = await request('reset', {});
        render();
        showMessage('Example data reset.');
    }
    catch (error) { showMessage(error.message, true); }
});

async function start() {
    try {
        if (location.protocol === 'file:') {
            throw new Error('Run run.bat and open http://localhost:8080 instead of opening this file directly.');
        }
        state = await request('state');
        render();
        element('connection').textContent = 'C++ backend connected';
    }
    catch (error) {
        element('connection').textContent = 'Backend unavailable';
        showMessage(error.message + ' Keep the server terminal open.', true);
    }
}
start();
